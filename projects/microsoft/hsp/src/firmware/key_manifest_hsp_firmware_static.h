// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef KEY_MANIFEST_HSP_FIRMWARE_STATIC_H_
#define KEY_MANIFEST_HSP_FIRMWARE_STATIC_H_

#include "firmware/key_manifest_hsp_firmware.h"


/* Internal functions declared to allow for static initialization. */
int key_manifest_hsp_firmware_verify (const struct key_manifest *manifest,
	const struct hash_engine *hash);
int key_manifest_hsp_firmware_is_allowed (const struct key_manifest *manifest);
int key_manifest_hsp_firmware_revokes_old_manifest (const struct key_manifest *manifest);
int key_manifest_hsp_firmware_update_revocation (const struct key_manifest *manifest);
const struct key_manifest_public_key* key_manifest_hsp_firmware_get_root_key (
	const struct key_manifest *manifest);
const struct key_manifest_public_key* key_manifest_hsp_firmware_get_app_key (
	const struct key_manifest *manifest);
const struct key_manifest_public_key* key_manifest_hsp_firmware_get_manifest_key (
	const struct key_manifest *manifest);


/**
 * Constant initializer for the key manifest base API.
 */
#define	KEY_MANIFEST_HSP_FIRMWARE_API_INIT  { \
		.verify = key_manifest_hsp_firmware_verify, \
		.is_allowed = key_manifest_hsp_firmware_is_allowed, \
		.revokes_old_manifest = key_manifest_hsp_firmware_revokes_old_manifest, \
		.update_revocation = key_manifest_hsp_firmware_update_revocation, \
		.get_root_key = key_manifest_hsp_firmware_get_root_key, \
		.get_app_key = key_manifest_hsp_firmware_get_app_key, \
		.get_manifest_key = key_manifest_hsp_firmware_get_manifest_key \
	}

/**
 * Static accessor for a specific key index within the key manifest.
 *
 * There is no checking index bounds or whether the key slot contains valid key data, so use with
 * caution.
 *
 * @param keys_ptr Memory location containing the key manifest data.
 * @param i Key index for the key to retrieve.
 */
#define	KEY_MANIFEST_HSP_FIRMWARE_KEY_DER(keys_ptr, i)		(keys_ptr)->fw_key[i]

/**
 * Constant initializer for the public key wrappers in the key manifest.
 *
 * @param keys_ptr Buffer for the key manifest data.
 * @param i Key index for the public key.
 */
#define	KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, i)	{ \
		.type = KEY_MANIFEST_ECC_DER_REF_KEY, \
		.key = { \
			.ecc_der_ref = { \
				.der = KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (keys_ptr, i), \
				.length = KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH \
			} \
		} \
	}


/**
 * Initialize a static instance of a key manifest parser for HSP firmware to use for run-time image
 * validation.  This does not load the manifest key data.  That will need to be initialized
 * separately with key_manifest_hsp_firmware_init_keys or
 * key_manifest_hsp_firmware_init_keys_from_memory.
 *
 * There is no validation done on the arguments.
 *
 * @param keys_ptr Buffer for the key manifest data.  If the key manifest will be loaded from flash,
 * this must be a writable location.
 * @param rot_ptr Interface to the HW RoT state.  This can be a constant instance.
 * @param security_ptr Manager for the device security policy to use during manifest verification.
 * This can be a constant instance.
 * @param ecdsa_ptr Signature verification context for verifying ECDSA signatures on the manifest.
 * This can be a constant instance.
 * @param root_key_ptr DER encoded public key to use for manifest verification.
 * @param der_length Length of the DER encoded public key.
 * @param secondary_key_ptr Optional DER encoded public key to use for verification of the secondary
 * manifest signature.  If this key is provided, the manifest must have a valid secondary signature.
 * @param secondary_length Length of the DER encoded secondary public key.
 */
#define	key_manifest_hsp_firmware_static_init(keys_ptr, rot_ptr, security_ptr, ecdsa_ptr, \
	root_key_ptr, der_length, secondary_key_ptr, secondary_length)	{ \
		.base = KEY_MANIFEST_HSP_FIRMWARE_API_INIT, \
		.keys = keys_ptr, \
		.rot = rot_ptr, \
		.security = security_ptr, \
		.ecdsa = ecdsa_ptr, \
		.static_keys = true, \
		.root_key = { \
			.type = KEY_MANIFEST_ECC_DER_REF_KEY, \
			.key = { \
				.ecc_der_ref = { \
					.der = root_key_ptr, \
					.length = der_length \
				} \
			} \
		}, \
		.secondary_key = { \
			.type = KEY_MANIFEST_ECC_DER_REF_KEY, \
			.key = { \
				.ecc_der_ref = { \
					.der = secondary_key_ptr, \
					.length = secondary_length \
				} \
			} \
		}, \
		.public_key = { \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 0), \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 1), \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 2), \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 3), \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 4), \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 5), \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 6), \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 7), \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 8), \
			KEY_MANIFEST_HSP_FIRMWARE_PUBLIC_KEY_INIT(keys_ptr, 9), \
		} \
	}


#endif	/* KEY_MANIFEST_HSP_FIRMWARE_STATIC_H_ */
