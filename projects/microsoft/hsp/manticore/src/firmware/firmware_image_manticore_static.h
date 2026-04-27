// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIRMWARE_IMAGE_MANTICORE_STATIC_H_
#define FIRMWARE_IMAGE_MANTICORE_STATIC_H_

#include "crypto/signature_verification_ecc_static.h"
#include "firmware/firmware_image_manticore.h"
#include "firmware/key_manifest_hsp_firmware_static.h"
#include "firmware/key_manifest_hsp_rom_static.h"


/* Internal functions declared to allow for static initialization. */
int firmware_image_manticore_load (const struct firmware_image *fw, const struct flash *flash,
	uint32_t base_addr);
int firmware_image_manticore_verify (const struct firmware_image *fw,
	const struct hash_engine *hash);
int firmware_image_manticore_get_image_size (const struct firmware_image *fw);
const struct key_manifest* firmware_image_manticore_get_key_manifest (
	const struct firmware_image *fw);
const struct firmware_header* firmware_image_manticore_get_firmware_header (
	const struct firmware_image *fw);


int firmware_image_manticore_is_not_impactful (const struct impactful_check *impactful);
int firmware_image_manticore_is_authorization_allowed (const struct impactful_check *impactful);


/**
 * Constant initializer for the firmware image base API.
 */
#define	FIRMWARE_IMAGE_MANTICORE_API_INIT  { \
		.load = firmware_image_manticore_load, \
		.verify = firmware_image_manticore_verify, \
		.get_image_size = firmware_image_manticore_get_image_size, \
		.get_key_manifest = firmware_image_manticore_get_key_manifest, \
		.get_firmware_header = firmware_image_manticore_get_firmware_header \
	}

/**
 * Constant initializer for the impactful update checking API.
 */
#define	FIRMWARE_IMAGE_MANTICORE_IMPACTFUL_API_INIT	{ \
	.is_not_impactful = firmware_image_manticore_is_not_impactful, \
	.is_authorization_allowed = firmware_image_manticore_is_authorization_allowed, \
}


/**
 * Initialize a static instance of a firmware image handler for Manticore firmware.
 *
 * There is no validation done on the arguments.
 *
 * @param fw_ptr The firmware image handler to initialize.
 * @param state_ptr Variable context for the firmware image.
 * @param hash_ptr Hash engine to use in contexts for which a hash engine is not otherwise provided.
 * @param ecc_ptr ECC interface to use for verification of main firmware components.
 * @param pka_ptr ECC interface to use for verification of 1SP firmware components.
 * @param ccs_ptr Interface to the CCS for HMAC operations on the firmware package.
 * @param rot_1sp_ptr RoT handler for the 1SP firmware.
 * @param rot_manifest_ptr RoT handler for the firmware key manifest.
 * @param security_ptr Manager for the device security policy that will be used during image
 * verification.
 * @param running_img_ptr The firmware descriptor for the firmware image currently present in memory
 * and executing.
 * @param fips_1sp_ptr A flag indicating whether the 1SP for the current execution context was FIPS
 * certified or not.
 */
#define	firmware_image_manticore_static_init(fw_ptr, state_ptr, hash_ptr, ecc_ptr, pka_ptr, \
	ccs_ptr, rot_1sp_ptr, rot_manifest_ptr, security_ptr, running_img_ptr, fips_1sp_ptr)	{ \
		.base = FIRMWARE_IMAGE_MANTICORE_API_INIT, \
		.base_impactful = FIRMWARE_IMAGE_MANTICORE_IMPACTFUL_API_INIT, \
		.state = state_ptr, \
		.hash = hash_ptr, \
		.pka = pka_ptr, \
		.ccs = ccs_ptr, \
		.security = security_ptr, \
		.running_img = running_img_ptr, \
		.fips_1sp = fips_1sp_ptr, \
		.manifest_1sp = key_manifest_hsp_rom_static_init (&(state_ptr)->manifest_1sp_state, \
			rot_1sp_ptr, pka_ptr), \
		.manifest = key_manifest_hsp_firmware_static_init (&(state_ptr)->fw_keys, \
			rot_manifest_ptr, security_ptr, &(fw_ptr)->verification.base, (state_ptr)->key_1sp, \
			sizeof ((state_ptr)->key_1sp), NULL, 0), \
		.verification = signature_verification_ecc_static_init (&(state_ptr)->verify_state, \
			ecc_ptr) \
	}


#endif	/* FIRMWARE_IMAGE_MANTICORE_STATIC_H_ */
