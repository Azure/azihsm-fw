// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_FW_KEYS_H_
#define MANTICORE_FW_KEYS_H_


/**
 * Indicies of keys available in the the Firmware Key Manifest of a Manticore image that firmware
 * can use for various operations that require authentication during run-time.
 */
enum {
	MANTICORE_FW_KEYS_APPLICATION_KEY = 0,	/**< The key used to verify firmware images. */
	MANTICORE_FW_KEYS_MANIFEST_ROOT_KEY,	/**< The key used to verify manifest verification keys. */
	MANTICORE_FW_KEYS_DEBUG_UNLOCK_KEY,		/**< The key used to verify unlock tokens. */
	MANTICORE_FW_KEYS_CLEAR_MANIFEST_KEY,	/**< The key used to authorize manifest clear operations. */
	MANTICORE_FW_KEYS_INTRUSION_RESET_KEY,	/**< The key used to authorize intrusion reset operations. */
	MANTICORE_FW_KEYS_ID_RENEWAL_KEY,		/**< The key used to authorize identity renewal operations. */
	MANTICORE_FW_KEYS_FIRMWARE_UPDATE_KEY,	/**< The key used to authorize firmware updates. */
	MANTICORE_FW_KEYS_RMA_KEY,				/**< The key used to authorize RETEST transitions. */
};


/**
 * Bit mask of key indicies that are required for correct operation of the firmware image.  This bit
 * mask can be used with the Firmware Key Manifest API to check for presence of these keys.
 *
 * Do not require the RMA key to be present as part of these checks.  Older key manifests do not
 * have this key, which would cause update issues to those versions of firmware.  Leave this check
 * to be handled at run-time by the firmware that is using the key.
 */
#define	MANTICORE_FW_KEYS_REQUIRED_KEYS \
	((1U << MANTICORE_FW_KEYS_APPLICATION_KEY) | (1U << MANTICORE_FW_KEYS_MANIFEST_ROOT_KEY) | \
	(1U << MANTICORE_FW_KEYS_DEBUG_UNLOCK_KEY) | (1U << MANTICORE_FW_KEYS_CLEAR_MANIFEST_KEY) | \
	(1U << MANTICORE_FW_KEYS_INTRUSION_RESET_KEY) | (1U << MANTICORE_FW_KEYS_ID_RENEWAL_KEY) | \
	(1U << MANTICORE_FW_KEYS_FIRMWARE_UPDATE_KEY))


#endif	/* MANTICORE_FW_KEYS_H_ */
