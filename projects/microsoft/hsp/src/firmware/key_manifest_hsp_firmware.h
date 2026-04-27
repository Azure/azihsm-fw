// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef KEY_MANIFEST_HSP_FIRMWARE_H_
#define KEY_MANIFEST_HSP_FIRMWARE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "crypto/signature_verification.h"
#include "firmware/hw_rot.h"
#include "firmware/key_manifest.h"
#include "flash/flash.h"
#include "system/security_manager.h"


/**
 * Magic number identifying a key manifest.
 */
#define	KEY_MANIFEST_HSP_FIRMWARE_MANIFEST_MARKER	0x46574b4d

/**
 * The number of key slots in the firmware key manifest.
 */
#define	KEY_MANIFEST_HSP_FIRMWARE_KEY_SLOTS			10

/**
 * Length of the DER encoded key in each key slot.
 */
#define	KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH	ECC_DER_P384_PUBLIC_LENGTH


#pragma pack(push,1)
/**
 * The Firmware Key Manifest structure that contains keys for run-time image verification ond other
 * firmware use.
 */
struct key_manifest_hsp_firmware_manifest {
	uint32_t marker;										/**< Marker identifying a key manifest. */
	uint8_t signature[ECC_DER_P384_ECDSA_MAX_LENGTH];		/**< Signature of the signed manifest header. */
	uint8_t secondary_sig[ECC_DER_P384_ECDSA_MAX_LENGTH];	/**< Secondary signature of the signed manifest header. */
	struct {
		uint64_t svn;										/**< Anti-rollback value for the manifest. */
		uint32_t length;									/**< Length of the manifest data. */
		uint8_t digest[SHA384_HASH_LENGTH];					/**< SHA2-384 digest of the manifest data. */
	} header_signed;										/**< Signed manifest header. */

	/** Keys contained in the manifest for firmware use. */
	uint8_t fw_key[KEY_MANIFEST_HSP_FIRMWARE_KEY_SLOTS][KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH];
};

#pragma pack(pop)

/**
 * Handler for the key manifest parsed by SP firmware to load and validate the run-time image.
 */
struct key_manifest_hsp_firmware {
	struct key_manifest base;								/**< Base key manifest instance. */
	const struct key_manifest_hsp_firmware_manifest *keys;	/**< The key manifest data. */
	const struct hw_rot *rot;								/**< Interface to the RoT state. */
	const struct security_manager *security;				/**< Manager for the device security policy. */
	const struct signature_verification *ecdsa;				/**< ECDSA signature verification engine. */
	struct key_manifest_public_key root_key;				/**< Root key for manifest verification. */
	struct key_manifest_public_key secondary_key;			/**< Secondary root key for manifest verification. */
	bool static_keys;										/**< Flag indicating the key manifest is a static buffer. */

	/** Public keys contained in the manifest. */
	struct key_manifest_public_key public_key[KEY_MANIFEST_HSP_FIRMWARE_KEY_SLOTS];
};


int key_manifest_hsp_firmware_init (struct key_manifest_hsp_firmware *manifest,
	struct key_manifest_hsp_firmware_manifest *key_data, const struct hw_rot *rot,
	const struct security_manager *security, const struct signature_verification *ecdsa,
	const uint8_t *root_key, size_t der_length, const uint8_t *secondary_key,
	size_t secondary_length, const struct flash *flash, uint32_t base_addr);
int key_manifest_hsp_firmware_init_from_memory (struct key_manifest_hsp_firmware *manifest,
	const struct key_manifest_hsp_firmware_manifest *key_data, const struct hw_rot *rot,
	const struct security_manager *security, const struct signature_verification *ecdsa,
	const uint8_t *root_key, size_t der_length, const uint8_t *secondary_key,
	size_t secondary_length);

int key_manifest_hsp_firmware_init_api (struct key_manifest_hsp_firmware *manifest,
	struct key_manifest_hsp_firmware_manifest *key_data, const struct hw_rot *rot,
	const struct security_manager *security, const struct signature_verification *ecdsa,
	const uint8_t *root_key, size_t der_length, const uint8_t *secondary_key,
	size_t secondary_length);

int key_manifest_hsp_firmware_init_keys (const struct key_manifest_hsp_firmware *manifest,
	const struct flash *flash, uint32_t base_addr);
int key_manifest_hsp_firmware_init_keys_from_memory (
	const struct key_manifest_hsp_firmware *manifest);

void key_manifest_hsp_firmware_release (const struct key_manifest_hsp_firmware *manifest);

uint64_t key_manifest_hsp_firmware_get_svn (const struct key_manifest_hsp_firmware *manifest);

const struct key_manifest_public_key* key_manifest_hsp_firmware_get_public_key (
	const struct key_manifest_hsp_firmware *manifest, int key_index);


int key_manifest_hsp_firmware_check_public_keys (const struct key_manifest_hsp_firmware *manifest,
	uint16_t key_mask);


#endif	/* KEY_MANIFEST_HSP_FIRMWARE_H_ */
