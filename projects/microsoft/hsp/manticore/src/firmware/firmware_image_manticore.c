// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "firmware_image_manticore.h"
#include "manticore_fw_keys.h"
#include "common/buffer_util.h"
#include "common/type_cast.h"
#include "common/unused.h"


/**
 * Release any loaded image data.  If no image data has been loaded, this does nothing.
 *
 * @param fw The image whose data should be release.
 */
static void firmware_image_manticore_release_loaded_image (
	const struct firmware_image_manticore *fw)
{
	if (fw->state->flash) {
		key_manifest_hsp_rom_release (&fw->manifest_1sp);
		hsp_fw_1sp_release (&fw->state->fw_1sp);
		manticore_firmware_package_release (&fw->state->fw_pkg);
		buffer_zeroize (&fw->state->key_1sp, sizeof (fw->state->key_1sp));

		fw->state->flash = NULL;
	}
}

int firmware_image_manticore_load (const struct firmware_image *fw, const struct flash *flash,
	uint32_t base_addr)
{
	const struct firmware_image_manticore *manticore = (const struct firmware_image_manticore*) fw;
	const struct key_manifest_public_key *key_1sp;
	const struct key_manifest_public_key *key_pkg;
	size_t offset;
	int status;

	if ((manticore == NULL) || (flash == NULL)) {
		return FIRMWARE_IMAGE_INVALID_ARGUMENT;
	}

	firmware_image_manticore_release_loaded_image (manticore);

	status = manticore->manifest_1sp.init_state (&manticore->manifest_1sp, flash, base_addr);
	if (status != 0) {
		return status;
	}

	offset = manticore->manifest_1sp.get_total_size (&manticore->manifest_1sp);

	status = hsp_fw_1sp_init (&manticore->state->fw_1sp, &manticore->state->fw_1sp_state, NULL,
		flash, base_addr + offset);
	if (status != 0) {
		goto free_1sp_manifest;
	}

	offset += hsp_fw_1sp_get_total_length (&manticore->state->fw_1sp);

	/* The root key for the FW manifest is the 1SP image key, but it needs to be DER encoded.  Even
	 * if the 1SP image is dual signed, the FW manifest will only be signed with the primary 1SP
	 * key. */
	key_1sp = manticore->manifest_1sp.base.get_app_key (&manticore->manifest_1sp.base);
	status = ecc_der_encode_public_key (key_1sp->key.ecc->x, key_1sp->key.ecc->y,
		key_1sp->key.ecc->key_length, manticore->state->key_1sp,
		sizeof (manticore->state->key_1sp));
	if (ROT_IS_ERROR (status)) {
		goto free_1sp;
	}

	status = key_manifest_hsp_firmware_init_keys (&manticore->manifest, flash, base_addr + offset);
	if (status != 0) {
		goto free_1sp;
	}

	status = key_manifest_hsp_firmware_check_public_keys (&manticore->manifest,
		MANTICORE_FW_KEYS_REQUIRED_KEYS);
	if (status != 0) {
		goto free_1sp;
	}

	offset += sizeof (manticore->state->fw_keys);

	/* The image key for the FW package needs to be loaded into the verification instance so the
	 * firmware descriptor can be loaded into memory. */
	key_pkg = manticore->manifest.base.get_app_key (&manticore->manifest.base);
	status = manticore->verification.base.set_verification_key (&manticore->verification.base,
		key_pkg->key.ecc_der_ref.der, key_pkg->key.ecc_der_ref.length);
	if (status != 0) {
		goto free_1sp;
	}

	status = manticore_firmware_package_init_require_sp (&manticore->state->fw_pkg,
		&manticore->state->fw_descriptor, flash, base_addr + offset, manticore->hash,
		&manticore->verification.base, manticore->ccs);

	manticore->verification.base.set_verification_key (&manticore->verification.base, NULL, 0);
	if (status != 0) {
		if (status == SIG_VERIFICATION_BAD_SIGNATURE) {
			status = FIRMWARE_IMAGE_BAD_SIGNATURE;
		}

		goto free_1sp;
	}

	manticore->state->flash = flash;

	return 0;

free_1sp:
	hsp_fw_1sp_release (&manticore->state->fw_1sp);
free_1sp_manifest:
	key_manifest_hsp_rom_release (&manticore->manifest_1sp);

	return status;
}

int firmware_image_manticore_verify (const struct firmware_image *fw,
	const struct hash_engine *hash)
{
	const struct firmware_image_manticore *manticore = (const struct firmware_image_manticore*) fw;
	const struct key_manifest_public_key *key_1sp;
	const struct key_manifest_public_key *secondary_key_1sp;
	const struct key_manifest_public_key *key_pkg;
	int status;

	if ((manticore == NULL) || (hash == NULL)) {
		return FIRMWARE_IMAGE_INVALID_ARGUMENT;
	}

	if (manticore->state->flash == NULL) {
		return FIRMWARE_IMAGE_NOT_LOADED;
	}

	/* Verify the 1SP key manifest and firmware image. */
	status = manticore->manifest_1sp.base.verify (&manticore->manifest_1sp.base, hash);
	if (status != 0) {
		goto exit;
	}

	status = manticore->manifest_1sp.base.is_allowed (&manticore->manifest_1sp.base);
	if (status != 1) {
		if (status == 0) {
			status = FIRMWARE_IMAGE_MANIFEST_REVOKED;
		}

		goto exit;
	}

	key_1sp = manticore->manifest_1sp.base.get_app_key (&manticore->manifest_1sp.base);
	secondary_key_1sp = manticore->manifest_1sp.get_secondary_key (&manticore->manifest_1sp);

	status = hsp_fw_1sp_verify_full_image (&manticore->state->fw_1sp, hash, manticore->pka,
		key_1sp->key.ecc, (secondary_key_1sp) ? secondary_key_1sp->key.ecc : NULL,
		manticore->manifest_1sp.get_svn (&manticore->manifest_1sp));
	if (status != 0) {
		goto exit;
	}

	/* Verify the firmware key manifest and firmware package. */
	status = manticore->manifest.base.verify (&manticore->manifest.base, hash);
	if (status != 0) {
		goto exit;
	}

	status = manticore->manifest.base.is_allowed (&manticore->manifest.base);
	if (status != 1) {
		if (status == 0) {
			status = FIRMWARE_IMAGE_MANIFEST_REVOKED;
		}

		goto exit;
	}

	/* The image key for the FW package needs to be loaded into the verification instance before
	 * starting FW package verification. */
	key_pkg = manticore->manifest.base.get_app_key (&manticore->manifest.base);
	status = manticore->verification.base.set_verification_key (&manticore->verification.base,
		key_pkg->key.ecc_der_ref.der, key_pkg->key.ecc_der_ref.length);
	if (status != 0) {
		goto exit;
	}

	/* The package authentication key was initialized during image load.
	 *
	 * A more complete verification would involve verifying each component within the package, but
	 * checking the overall package signature gives us confidence that the package is authentic and
	 * one consistent unit.  Properly signed packages that are malformed (e.g. where individual
	 * components are not constructed and/or signed properly) will not be detected here.  This type
	 * of failure is unlikely and would be detected when trying to boot the package images. */
	status = manticore_firmware_package_verify_package_on_flash (&manticore->state->fw_pkg, hash,
		&manticore->verification.base);
	manticore->verification.base.set_verification_key (&manticore->verification.base, NULL, 0);
	if (status != 0) {
		goto exit;
	}

	/* The FW package must have the same SVN as the FW key manifest. */
	if (manticore_firmware_descriptor_get_svn (&manticore->state->fw_descriptor) !=
		key_manifest_hsp_firmware_get_svn (&manticore->manifest)) {
		status = FIRMWARE_IMAGE_REVOKED;
	}

exit:
	/* Make sure any type of verification failures get reported out correctly. */
	if ((status == ECC_HW_ECDSA_BAD_SIGNATURE) || (status == ECC_ENGINE_BAD_SIGNATURE) ||
		(status == SIG_VERIFICATION_BAD_SIGNATURE) || (status == KEY_MANIFEST_VERIFY_FAILED) ||
		(status == HSP_FW_1SP_IMAGE_VERIFY_FAILED)) {
		status = FIRMWARE_IMAGE_BAD_SIGNATURE;
	}

	return status;
}

int firmware_image_manticore_get_image_size (const struct firmware_image *fw)
{
	const struct firmware_image_manticore *manticore = (const struct firmware_image_manticore*) fw;

	if (manticore == NULL) {
		return FIRMWARE_IMAGE_INVALID_ARGUMENT;
	}

	if (manticore->state->flash == NULL) {
		return FIRMWARE_IMAGE_NOT_LOADED;
	}

	return manticore->manifest_1sp.get_total_size (&manticore->manifest_1sp) +
		   hsp_fw_1sp_get_total_length (&manticore->state->fw_1sp) +
		   sizeof (manticore->state->fw_keys) +
		   manticore_firmware_package_get_length (&manticore->state->fw_pkg);
}

const struct key_manifest* firmware_image_manticore_get_key_manifest (
	const struct firmware_image *fw)
{
	const struct firmware_image_manticore *manticore = (const struct firmware_image_manticore*) fw;

	if (manticore == NULL) {
		return NULL;
	}

	if (manticore->state->flash == NULL) {
		return NULL;
	}

	return &manticore->manifest.base;
}

const struct firmware_header* firmware_image_manticore_get_firmware_header (
	const struct firmware_image *fw)
{
	const struct firmware_image_manticore *manticore = (const struct firmware_image_manticore*) fw;

	if (manticore == NULL) {
		return NULL;
	}

	if (manticore->state->flash == NULL) {
		return NULL;
	}

	return manticore_firmware_package_get_firmware_header (&manticore->state->fw_pkg);
}

int firmware_image_manticore_is_not_impactful (const struct impactful_check *impactful)
{
	const struct firmware_image_manticore *manticore =
		TO_DERIVED_TYPE (impactful, const struct firmware_image_manticore, base_impactful);
	uint16_t compat_running;
	uint16_t compat_update;
	uint8_t fips_running;
	uint8_t fips_update;
	bool fips_1sp_update;

	if (impactful == NULL) {
		return FIRMWARE_IMAGE_INVALID_ARGUMENT;
	}

	if (manticore->state->flash == NULL) {
		return FIRMWARE_IMAGE_NOT_LOADED;
	}

	/* Check for the same firmware image compatibility version between the running image and the new
	 * image. */
	compat_running =
		manticore_firmware_descriptor_image_compatibility_version (manticore->running_img);

	compat_update =
		manticore_firmware_descriptor_image_compatibility_version (
		&manticore->state->fw_descriptor);

	if (compat_running != compat_update) {
		return FIRMWARE_IMAGE_INCOMPATIBLE;
	}

	/* Check for the same FIPS certification state between the running image and the new image.
	 * Switching between FIPS approved and non-approved mode needs to involve an SoC reset.
	 *
	 * Keep this check simple by only comparing the values reported in the firmware descriptor
	 * without considering the unlock state of the device.  This is acceptable since unlocked
	 * devices will always be in FIPS non-approved mode, regardless of the firmware certification,
	 * and it takes a SoC reset to return to the locked state.  The worst-case scenario by
	 * simplifying this check is that an impactful update is unnecessarily required. */
	fips_running = manticore_firmware_descriptor_fips_certified (manticore->running_img);
	fips_update = manticore_firmware_descriptor_fips_certified (&manticore->state->fw_descriptor);

	if (fips_running != fips_update) {
		return FIRMWARE_IMAGE_INCOMPATIBLE;
	}

	/* The 1SP also needs to be checked for a change in the FIPS certification state.  If the update
	 * contains a dual-signed 1SP, then it is considered FIPS certified. */
	fips_1sp_update =
		(manticore->manifest_1sp.get_secondary_key (&manticore->manifest_1sp) != NULL);

	if (*manticore->fips_1sp != fips_1sp_update) {
		return FIRMWARE_IMAGE_INCOMPATIBLE;
	}

	return 0;
}

int firmware_image_manticore_is_authorization_allowed (const struct impactful_check *impactful)
{
	/* Never block impactful updates. */
	UNUSED (impactful);

	return 0;
}

/**
 * Initialize a firmware image handler for Manticore firmware on flash.
 *
 * @param fw The firmware image handler to initialize.
 * @param state Variable context for the firmware image.  This must be uninitialized.
 * @param hash Hash engine to use in contexts for which a hash engine is not otherwise provided.
 * @param ecc ECC interface to use for verification of main firmware components.
 * @param pka ECC interface to use for verification of 1SP firmware components.
 * @param ccs Interface to the CCS for HMAC operations on the firmware package.
 * @param rot_1sp RoT handler for the 1SP firmware.
 * @param rot_manifest RoT handler for the firmware key manifest.
 * @param security Manager for the device security policy that will be used during image
 * verification.
 * @param running_img The firmware descriptor for the firmware image currently present in memory and
 * executing.
 * @param fips_1sp A flag indicating whether the 1SP for the current execution context was FIPS
 * certified or not.
 *
 * @return 0 if the firmware image handler was successfully initialized or an error code.
 */
int firmware_image_manticore_init (struct firmware_image_manticore *fw,
	struct firmware_image_manticore_state *state, const struct hash_engine *hash,
	const struct ecc_engine *ecc, const struct ecc_hw *pka, const struct ccs_ksu_interface *ccs,
	const struct hw_rot *rot_1sp, const struct hw_rot *rot_manifest,
	const struct security_manager *security,
	const struct manticore_firmware_descriptor *running_img, const bool *fips_1sp)
{
	if ((fw == NULL) || (state == NULL) || (hash == NULL) || (ecc == NULL) || (pka == NULL) ||
		(ccs == NULL) || (rot_1sp == NULL) || (rot_manifest == NULL) || (security == NULL) ||
		(running_img == NULL) || (fips_1sp == NULL)) {
		return FIRMWARE_IMAGE_INVALID_ARGUMENT;
	}

	memset (fw, 0, sizeof (struct firmware_image_manticore));
	memset (state, 0, sizeof (struct firmware_image_manticore_state));

	/* These init calls can't fail in this context. */
	key_manifest_hsp_rom_init_api (&fw->manifest_1sp, &state->manifest_1sp_state, rot_1sp, pka);
	signature_verification_ecc_init (&fw->verification, &state->verify_state, ecc, NULL, 0);

	/* The root key contents don't matter until we are ready to run verification, but the buffer
	 * needs to be valid now. */
	key_manifest_hsp_firmware_init_api (&fw->manifest, &state->fw_keys, rot_manifest, security,
		&fw->verification.base, state->key_1sp, sizeof (state->key_1sp), NULL, 0);

	fw->base.load = firmware_image_manticore_load;
	fw->base.verify = firmware_image_manticore_verify;
	fw->base.get_image_size = firmware_image_manticore_get_image_size;
	fw->base.get_key_manifest = firmware_image_manticore_get_key_manifest;
	fw->base.get_firmware_header = firmware_image_manticore_get_firmware_header;

	fw->base_impactful.is_not_impactful = firmware_image_manticore_is_not_impactful;
	fw->base_impactful.is_authorization_allowed = firmware_image_manticore_is_authorization_allowed;

	fw->state = state;
	fw->hash = hash;
	fw->pka = pka;
	fw->ccs = ccs;
	fw->security = security;
	fw->running_img = running_img;
	fw->fips_1sp = fips_1sp;

	return 0;
}

/**
 * Initialize only the variable state for a Manticore firmware image.  The rest of the handler is
 * assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param fw The firmware image handler that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int firmware_image_manticore_init_state (const struct firmware_image_manticore *fw)
{
	if ((fw == NULL) || (fw->state == NULL) || (fw->hash == NULL) || (fw->pka == NULL) ||
		(fw->ccs == NULL) || (fw->security == NULL) || (fw->running_img == NULL) ||
		(fw->fips_1sp == NULL)) {
		return FIRMWARE_IMAGE_INVALID_ARGUMENT;
	}

	memset (fw->state, 0, sizeof (struct firmware_image_manticore_state));

	return signature_verification_ecc_init_state (&fw->verification, NULL, 0);
}

/**
 * Release the resources used by a Manticore firmware image.
 *
 * @param fw The firmware image handler to release.
 */
void firmware_image_manticore_release (const struct firmware_image_manticore *fw)
{
	if (fw) {
		firmware_image_manticore_release_loaded_image (fw);
		key_manifest_hsp_firmware_release (&fw->manifest);
		signature_verification_ecc_release (&fw->verification);
	}
}
