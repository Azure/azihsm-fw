// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIRMWARE_IMAGE_MANTICORE_H_
#define FIRMWARE_IMAGE_MANTICORE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "asn1/ecc_der_util.h"
#include "crypto/signature_verification_ecc.h"
#include "firmware/firmware_image.h"
#include "firmware/hsp_fw_1sp.h"
#include "firmware/hsp_fw_1sp.h"
#include "firmware/impactful_check.h"
#include "firmware/key_manifest_hsp_firmware.h"
#include "firmware/key_manifest_hsp_rom.h"
#include "firmware/manticore_firmware_package.h"
#include "system/security_manager.h"


/**
 * Variable context for a Manticore firmware image handler.
 */
struct firmware_image_manticore_state {
	struct key_manifest_hsp_rom_state manifest_1sp_state;	/**< Variable context for the 1SP key manifest. */
	struct hsp_fw_1sp_state fw_1sp_state;					/**< Variable context for the 1SP firmware. */
	struct key_manifest_hsp_firmware_manifest fw_keys;		/**< Key manifest data for the main firmware. */
	struct signature_verification_ecc_state verify_state;	/**< Variable context for signature verification. */
	struct hsp_fw_1sp fw_1sp;								/**< 1SP firmware that will be loaded by ROM. */
	uint8_t key_1sp[ECC_DER_P384_PUBLIC_LENGTH];			/**< DER encoded public key used to sign the 1SP image. */
	struct manticore_firmware_descriptor fw_descriptor;		/**< Descriptor for the main firmware. */
	struct manticore_firmware_package fw_pkg;				/**< Main firmware image. */
	const struct flash *flash;								/**< Flash where the image is stored. */
};

/**
 * Handler for a Manticore firmware image stored on flash.
 */
struct firmware_image_manticore {
	struct firmware_image base;									/**< Base image interface. */
	struct impactful_check base_impactful;						/**< Base interface for impactful update checks. */
	struct firmware_image_manticore_state *state;				/**< Variable context for the image. */
	const struct hash_engine *hash;								/**< Hash engine for descriptor loading. */
	const struct ecc_hw *pka;									/**< ECC interface for 1SP firmware verification. */
	const struct ccs_ksu_interface *ccs;						/**< CCS interface for HMAC operations. */
	const struct security_manager *security;					/**< Manager for the device security policy. */
	const struct manticore_firmware_descriptor *running_img;	/**< Firmware descriptor for the currently loaded image. */
	const bool *fips_1sp;										/**< FIPS certification state of the current 1SP image. */
	struct key_manifest_hsp_rom manifest_1sp;					/**< Key manifest for the 1SP image. */
	struct key_manifest_hsp_firmware manifest;					/**< Key manifest for the main firmware. */
	struct signature_verification_ecc verification;				/**< Verification wrapper for main firmware. */
};


int firmware_image_manticore_init (struct firmware_image_manticore *fw,
	struct firmware_image_manticore_state *state, const struct hash_engine *hash,
	const struct ecc_engine *ecc, const struct ecc_hw *pka, const struct ccs_ksu_interface *ccs,
	const struct hw_rot *rot_1sp, const struct hw_rot *rot_manifest,
	const struct security_manager *security,
	const struct manticore_firmware_descriptor *running_img, const bool *fips_1sp);
int firmware_image_manticore_init_state (const struct firmware_image_manticore *fw);
void firmware_image_manticore_release (const struct firmware_image_manticore *fw);


#endif	/* FIRMWARE_IMAGE_MANTICORE_H_ */
