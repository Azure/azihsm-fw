// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "manticore_bootloader.h"
#include "manticore_fw_keys.h"
#include "common/buffer_util.h"


/**
 * Initialize a bootloader for loading Manticore firmware into all CPU cores.
 *
 * This will inspect the firmware image and verify security critical data.  If these checks are
 * successful, the overall package meets security requirements and the images are ready to be
 * loaded.  It will not be known if the images themselves are valid until they are loaded.
 *
 * @param boot The bootloader to initialize.
 * @param state Variable context for the bootloader.  This must be uninitialized.
 * @param flash The flash device that contains the firmware to load.
 * @param base_addr Address on flash to the beginning of the firmware data.
 * @param fw_keys Storage for the firmware key manifest of the image.
 * @param fw_descriptor Storage for the firmware descriptor of the image.
 * @param hash Hash engine to use for firmware verification.
 * @param ecc ECC interface to use for verification of firmware signatures.
 * @param ccs CCS interface for validating information about encrypted images.
 * @param key_1sp DER encoded public key that was used to verify the 1SP firmware image.  This key
 * will be used to verify the firmware key manifest.
 * @param key_length Length of the DER encoded 1SP authentication key.
 * @param rot_manifest RoT handler for the firmware key manifest.
 * @param security Manager for the device security policy that should be applied while loading the
 * firmware images.
 * @param sp_loader Handler for loading SPRT firmware images into SP memory.
 * @param cp_loader Handler for loading CP firmware images into CP memory.
 * @param fp0_loader Handler for loading FP core 0 firmware images into FP0 memory.
 * @param fp1_loader Handler for loading FP core 1 firmware images into FP1 memory.
 * @param fp2_loader Handler for loading FP core 2 firmware images into FP2 memory.
 * @param pcie_loader Handler for loading PCIe PHY firmware images into memory.
 *
 * @return 0 if the bootloader was successfully initialized or an error code.
 */
int manticore_bootloader_init (struct manticore_bootloader *boot,
	struct manticore_bootloader_state *state, const struct flash *flash, uint32_t base_addr,
	struct key_manifest_hsp_firmware_manifest *fw_keys,
	struct manticore_firmware_descriptor *fw_descriptor, const struct hash_engine *hash,
	const struct ecc_engine *ecc, const struct ccs_ksu_interface *ccs, const uint8_t *key_1sp,
	size_t key_length, const struct hw_rot *rot_manifest, const struct security_manager *security,
	const struct firmware_loader *sp_loader, const struct firmware_loader *cp_loader,
	const struct firmware_loader *fp0_loader, const struct firmware_loader *fp1_loader,
	const struct firmware_loader *fp2_loader, const struct firmware_loader *pcie_loader)
{
	int status;

	if ((boot == NULL) || (state == NULL) || (fw_keys == NULL)) {
		return MANTICORE_BOOTLOADER_INVALID_ARGUMENT;
	}

	memset (boot, 0, sizeof (struct manticore_bootloader));

	status = signature_verification_ecc_init_api (&boot->verification, &state->verify_state, ecc);
	if (status != 0) {
		return status;
	}

	status = key_manifest_hsp_firmware_init_api (&boot->manifest, fw_keys, rot_manifest, security,
		&boot->verification.base, key_1sp, key_length, NULL, 0);
	if (status != 0) {
		return status;
	}

	boot->state = state;
	boot->flash = flash;
	boot->hash = hash;
	boot->security = security;
	boot->sp_loader = sp_loader;
	boot->cp_loader = cp_loader;
	boot->fp0_loader = fp0_loader;
	boot->fp1_loader = fp1_loader;
	boot->fp2_loader = fp2_loader;
	boot->pcie_loader = pcie_loader;

	return manticore_bootloader_init_state (boot, base_addr, ccs, rot_manifest, fw_descriptor);
}

/**
 * Initialize only the variable state for a Manticore main bootloader.  The rest of the instance is
 * assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * This will inspect the firmware image and verify security critical data.  If these checks are
 * successful, the overall package meets security requirements and the images are ready to be
 * loaded.  It will not be known if the images themselves are valid until they are loaded.
 *
 * @param boot The firmware bootloader that contains the state to initialize.
 * @param base_addr Address on flash to the beginning of the firmware data.
 * @param ccs CCS interface for validating information about encrypted images.
 * @param rot_manifest RoT handler for the firmware key manifest.
 * @param fw_descriptor Storage for the firmware descriptor of the image.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int manticore_bootloader_init_state (const struct manticore_bootloader *boot, uint32_t base_addr,
	const struct ccs_ksu_interface *ccs, const struct hw_rot *rot_manifest,
	struct manticore_firmware_descriptor *fw_descriptor)
{
	uint32_t offset;
	const struct security_policy *policy;
	const struct key_manifest_public_key *key_1sp;
	const struct key_manifest_public_key *pkg_key;
	int status;

	if ((boot == NULL) || (ccs == NULL) || (rot_manifest == NULL) || (fw_descriptor == NULL) ||
		(boot->state == NULL) || (boot->flash == NULL) || (boot->hash == NULL) ||
		(boot->security == NULL) || (boot->sp_loader == NULL) || (boot->cp_loader == NULL) ||
		(boot->fp0_loader == NULL) || (boot->fp1_loader == NULL) || (boot->fp2_loader == NULL) ||
		(boot->pcie_loader == NULL)) {
		return MANTICORE_BOOTLOADER_INVALID_ARGUMENT;
	}

	memset (boot->state, 0, sizeof (struct manticore_bootloader_state));

	status = signature_verification_ecc_init_state (&boot->verification, NULL, 0);
	if (status != 0) {
		return status;
	}

	/* No need to load or verify the 1SP components, but need to get the lengths to determine the
	 * flash address of the main firmware pieces. */
	status = key_manifest_hsp_rom_get_size_on_flash (boot->flash, base_addr);
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	offset = status;

	status = hsp_fw_1sp_get_size_on_flash (boot->flash, base_addr + offset);
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	offset += status;

	/* Load and verify the firmware key manifest.  Ensure the manifest has not been revoked. */
	status = key_manifest_hsp_firmware_init_keys (&boot->manifest, boot->flash, base_addr + offset);
	if (status != 0) {
		return status;
	}

	status = key_manifest_hsp_firmware_check_public_keys (&boot->manifest,
		MANTICORE_FW_KEYS_REQUIRED_KEYS);
	if (status != 0) {
		goto free_manifest;
	}

	policy = security_manager_get_security_policy (boot->security);

	/* Verification of the firmware key manifest does not include root key verification, so this
	 * step must be done here.  This check will be skipped if the security policy doesn't require
	 * trusted firmware signing. */
	if (security_policy_enforce_firmware_signing (policy)) {
		key_1sp = boot->manifest.base.get_root_key (&boot->manifest.base);

		status = rot_manifest->verify_root_key (rot_manifest, key_1sp->key.ecc_der_ref.der,
			key_1sp->key.ecc_der_ref.length, boot->hash);
		if ((status != 0) && (status != HW_ROT_NO_ROOT_KEY) && (status != HW_ROT_UNSUPPORTED)) {
			goto free_manifest;
		}
	}

	status = boot->manifest.base.verify (&boot->manifest.base, boot->hash);
	if (status != 0) {
		goto free_manifest;
	}

	status = boot->manifest.base.is_allowed (&boot->manifest.base);
	if (status != 1) {
		if (status == 0) {
			status = MANTICORE_BOOTLOADER_MANIFEST_REVOKED;
		}

		goto free_manifest;
	}

	offset += sizeof (struct key_manifest_hsp_firmware_manifest);

	/* Load and verify the firmware package descriptor and other metadata.  Ensure the package
	 * contains an SPRT image and has not been revoked. */
	pkg_key = boot->manifest.base.get_app_key (&boot->manifest.base);
	status = boot->verification.base.set_verification_key (&boot->verification.base,
		pkg_key->key.ecc_der_ref.der, pkg_key->key.ecc_der_ref.length);
	if (status != 0) {
		goto free_manifest;
	}

	status = manticore_firmware_package_init_require_sp (&boot->state->fw_pkg, fw_descriptor,
		boot->flash, base_addr + offset, boot->hash, &boot->verification.base, ccs);
	if (status != 0) {
		goto free_verify;
	}

	/* The firmware package must have the same SVN as the firmware key manifest. */
	if (manticore_firmware_descriptor_get_svn (fw_descriptor) !=
		key_manifest_hsp_firmware_get_svn (&boot->manifest)) {
		status = MANTICORE_BOOTLOADER_FIRMWARE_REVOKED;
		goto free_package;
	}

	return 0;

free_package:
	manticore_firmware_package_release (&boot->state->fw_pkg);
free_verify:
	signature_verification_ecc_release (&boot->verification);
free_manifest:
	key_manifest_hsp_firmware_release (&boot->manifest);

	return status;
}

/**
 * Release the resources used by a Manticore main firmware bootloader.
 *
 * @param boot The firmware bootloader to release.
 */
void manticore_bootloader_release (const struct manticore_bootloader *boot)
{
	if (boot) {
		manticore_firmware_package_release (&boot->state->fw_pkg);
		key_manifest_hsp_firmware_release (&boot->manifest);
		signature_verification_ecc_release (&boot->verification);
	}
}

/**
 * Load and verify all components of a single type.
 *
 * @param boot The firmware bootloader to execute.
 * @param no_images_error Error code indicating there are no images of that type.
 * @param hash_out Optional output for the aggregate measurement of the components loaded.  This
 * will be 0's if there are no images.
 * @param length Length of the output hash buffer.
 *
 * @return 0 if the components were loaded and verified successfully or an error code.
 */
static int manticore_bootloader_load_components (const struct manticore_bootloader *boot,
	int no_images_error, uint8_t *hash_out, size_t length)
{
	uint8_t *digests;
	size_t img_count;
	int status;

	switch (no_images_error) {
		case MANTICORE_FW_PACKAGE_NO_SP_IMAGES:
			status = manticore_firmware_package_load_sp_components (&boot->state->fw_pkg,
				boot->sp_loader, boot->hash, &boot->verification.base, &digests, &img_count);
			break;

		case MANTICORE_FW_PACKAGE_NO_CP_IMAGES:
			status = manticore_firmware_package_load_cp_components (&boot->state->fw_pkg,
				boot->cp_loader, boot->hash, &boot->verification.base, &digests, &img_count);
			break;

		case MANTICORE_FW_PACKAGE_NO_FP0_IMAGES:
			status = manticore_firmware_package_load_fp0_components (&boot->state->fw_pkg,
				boot->fp0_loader, boot->hash, &boot->verification.base, &digests, &img_count);
			break;

		case MANTICORE_FW_PACKAGE_NO_FP1_IMAGES:
			status = manticore_firmware_package_load_fp1_components (&boot->state->fw_pkg,
				boot->fp1_loader, boot->hash, &boot->verification.base, &digests, &img_count);
			break;

		case MANTICORE_FW_PACKAGE_NO_FP2_IMAGES:
			status = manticore_firmware_package_load_fp2_components (&boot->state->fw_pkg,
				boot->fp2_loader, boot->hash, &boot->verification.base, &digests, &img_count);
			break;

		case MANTICORE_FW_PACKAGE_NO_PCIE_IMAGES:
			status = manticore_firmware_package_load_pcie_components (&boot->state->fw_pkg,
				boot->pcie_loader, boot->hash, &boot->verification.base, &digests, &img_count);
			break;
	}
	if (status != 0) {
		if (hash_out && (status == no_images_error)) {
			memset (hash_out, 0, length);
		}

		return status;
	}

	if (hash_out) {
		status = boot->hash->calculate_sha384 (boot->hash, digests, SHA384_HASH_LENGTH * img_count,
			hash_out, length);
	}

	buffer_zeroize (digests, SHA384_HASH_LENGTH * img_count);
	platform_free (digests);

	return status;
}

/**
 * Load and verify only the SPRT firmware image.
 *
 * @param boot The firmware bootloader to execute.
 * @param hash_out Optional output for the aggregate measurement of the SPRT firmware.  This will be
 * a hash over the concatenated digests for all SPRT components.
 * @param length Length of the output hash buffer.
 *
 * @return 0 if all SP firmware components were successfully loaded and verified or an error code.
 */
int manticore_bootloader_load_sp_only (const struct manticore_bootloader *boot, uint8_t *hash_out,
	size_t length)
{
	if (boot == NULL) {
		return MANTICORE_BOOTLOADER_INVALID_ARGUMENT;
	}

	return manticore_bootloader_load_components (boot, MANTICORE_FW_PACKAGE_NO_SP_IMAGES, hash_out,
		length);
}

/**
 * Load and verify all firmware images into all CPU cores.
 *
 * @param boot The firmware bootloader to execute.
 * @param sp_hash_out Optional output for the aggregate measurement of the SPRT firmware.  This will
 * be a hash over the concatenated digests for all SPRT components.
 * @param sp_hash_length Length of the SPRT output hash buffer.
 * @param cp_hash_out Optional output for the aggregate measurement of the CP firmware.  This will
 * be a hash over the concatenated digests for all CP components.  This will be filled with 0's if
 * there are no CP components.
 * @param cp_hash_length Length of the CP output hash buffer.
 * @param fp0_hash_out Optional output for the aggregate measurement of the FP firmware.  This will
 * be a hash over the concatenated digests for all components for FP core 0.  This will be filled
 * with 0's if there are no FP core 0 components.
 * @param fp0_hash_length Length of the FP0 output hash buffer
 * @param fp1_hash_out Optional output for the aggregate measurement of the FP firmware.  This will
 * be a hash over the concatenated digests for all components for FP core 1.  This will be filled
 * with 0's if there are no FP core 1 components.
 * @param fp1_hash_length Length of the FP1 output hash buffer
 * @param fp2_hash_out Optional output for the aggregate measurement of the FP firmware.  This will
 * be a hash over the concatenated digests for all components for FP core 2.  This will be filled
 * with 0's if there are no FP core 2 components.
 * @param fp2_hash_length Length of the FP2 output hash buffer
 *
 * @return 0 if all CPU cores were successfully loaded and verified or an error code.
 */
int manticore_bootloader_load_all_cores (const struct manticore_bootloader *boot,
	uint8_t *sp_hash_out, size_t sp_hash_length, uint8_t *cp_hash_out, size_t cp_hash_length,
	uint8_t *fp0_hash_out, size_t fp0_hash_length, uint8_t *fp1_hash_out, size_t fp1_hash_length,
	uint8_t *fp2_hash_out, size_t fp2_hash_length)
{
	int status;

	if (boot == NULL) {
		return MANTICORE_BOOTLOADER_INVALID_ARGUMENT;
	}

	status = manticore_bootloader_load_components (boot, MANTICORE_FW_PACKAGE_NO_SP_IMAGES,
		sp_hash_out, sp_hash_length);
	if (status != 0) {
		return status;
	}

	status = manticore_bootloader_load_components (boot, MANTICORE_FW_PACKAGE_NO_CP_IMAGES,
		cp_hash_out, cp_hash_length);
	if ((status != 0) && (status != MANTICORE_FW_PACKAGE_NO_CP_IMAGES)) {
		return status;
	}

	status = manticore_bootloader_load_components (boot, MANTICORE_FW_PACKAGE_NO_FP0_IMAGES,
		fp0_hash_out, fp0_hash_length);
	if ((status != 0) && (status != MANTICORE_FW_PACKAGE_NO_FP0_IMAGES)) {
		return status;
	}

	status = manticore_bootloader_load_components (boot, MANTICORE_FW_PACKAGE_NO_FP1_IMAGES,
		fp1_hash_out, fp1_hash_length);
	if ((status != 0) && (status != MANTICORE_FW_PACKAGE_NO_FP1_IMAGES)) {
		return status;
	}

	status = manticore_bootloader_load_components (boot, MANTICORE_FW_PACKAGE_NO_FP2_IMAGES,
		fp2_hash_out, fp2_hash_length);
	if ((status != 0) && (status != MANTICORE_FW_PACKAGE_NO_FP2_IMAGES)) {
		return status;
	}

	return 0;
}

/**
 * Load and verify the PCIe PHY firmware image.
 *
 * @param boot The firmware bootloader to execute.
 * @param hash_out Optional output for the aggregate measurement of the PCIe PHY firmware.  This
 * will be a hash over the concatenated digests for all PCIe PHY components.
 * @param length Length of the output hash buffer.
 *
 * @return 0 if all PCIe PHY firmware components were successfully loaded and verified or an error
 * code.
 */
int manticore_bootloader_load_pcie_phy (const struct manticore_bootloader *boot, uint8_t *hash_out,
	size_t length)
{
	if (boot == NULL) {
		return MANTICORE_BOOTLOADER_INVALID_ARGUMENT;
	}

	return manticore_bootloader_load_components (boot, MANTICORE_FW_PACKAGE_NO_PCIE_IMAGES,
		hash_out, length);
}
