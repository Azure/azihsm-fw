// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include "load_image_1sp.h"
#include "rom_logging.h"
#include "logging/code_path_integrity.h"


/**
 * Secure execution tracing values.  Tracing at this level gives visibility into the sequence of
 * calls but is easier to manage in cases where something in the image load process fails.
 */
enum {
	LOAD_IMAGE_1SP_TRACE_START_FLASH_LOAD = 0x1000,				/**< Begin flash image loading. */
	LOAD_IMAGE_1SP_TRACE_START_MEMORY_LOAD = 0x1001,			/**< Begin memory image loading. */
	LOAD_IMAGE_1SP_TRACE_FLASH_MANIFEST = 0x2000,				/**< Loading a manifest from flash. */
	LOAD_IMAGE_1SP_TRACE_MEMORY_MANIFEST = 0x2001,				/**< Loading a manifest from memory. */
	LOAD_IMAGE_1SP_TRACE_FLASH_MANIFEST_VALIDATION = 0x3000,	/**< Validating a loaded manifest from flash. */
	LOAD_IMAGE_1SP_TRACE_MEMORY_MANIFEST_VALIDATION = 0x3001,	/**< Validating a loaded manifest from memory. */
	LOAD_IMAGE_1SP_TRACE_FLASH_IMAGE = 0x4000,					/**< Loading an image header from flash. */
	LOAD_IMAGE_1SP_TRACE_MEMORY_IMAGE = 0x4001,					/**< Loading an image header from memory. */
	LOAD_IMAGE_1SP_TRACE_FLASH_IMAGE_VALIDATION = 0x5000,		/**< Copy and validate a firmware image from flash. */
	LOAD_IMAGE_1SP_TRACE_MEMORY_IMAGE_VALIDATION = 0x5001,		/**< Copy and validate a firmware image from flash. */
	LOAD_IMAGE_1SP_TRACE_FLASH_LOAD_DONE = 0x6000,				/**< Done with the flash load and validate sequence. */
	LOAD_IMAGE_1SP_TRACE_MEMORY_LOAD_DONE = 0x6001,				/**< Done with the memory load and validate sequence. */
};


/**
 * Verify the key manifest for the firmware image and get the key that should be used to
 * authenticate the image.
 *
 * @param manifest The key manifest to verify.
 * @param hash The hash engine to use for manifest verification.
 * @param image_key Output for the key for image authentication.
 * @param secondary_key Output for a secondary key for image authentication.
 *
 * @return 0 if the manifest is valid or an error code.
 */
static int load_image_1sp_verify_manifest (const struct key_manifest_hsp_rom *manifest,
	const struct hash_engine *hash, const struct key_manifest_public_key **image_key,
	const struct key_manifest_public_key **secondary_key)
{
	int status;

	code_path_integrity_message_trace (ROM_LOGGING_TRACE_VERIFY_MANIFEST);

	status = manifest->base.verify (&manifest->base, hash);
	if (status != 0) {
		return status;
	}

	code_path_integrity_message_trace (ROM_LOGGING_TRACE_REVOCATION_CHECK);

	status = manifest->base.is_allowed (&manifest->base);
	if (status != 1) {
		return (status == 0) ? LOAD_IMAGE_1SP_MANIFEST_REVOKED : status;
	}

	*image_key = manifest->base.get_app_key (&manifest->base);
	if (*image_key == NULL) {
		return LOAD_IMAGE_1SP_NO_IMAGE_KEY;
	}

	if (!manifest->is_tenancy_grant (manifest)) {
		*secondary_key = manifest->get_secondary_key (manifest);
	}
	else {
		*secondary_key = NULL;
	}

	return 0;
}

/**
 * Load and verify the 1SP firmware image.  If there is a failure, the key manifest and image
 * instances will be released.
 *
 * @param image The 1SP firmware image to load and verify.
 * @param manifest The key manifest for the firmware image.
 * @param pka The PKA engine to use for signature verification.
 * @param hash A hash engine to use for image verification.
 * @param image_key The key to use for image authentication.
 * @param secondary_key An optional additional key to use for image authentication.
 *
 * @return 0 if the image was loaded and verified or an error code.
 */
static int load_image_1sp_verify_image (const struct hsp_fw_1sp *image,
	const struct key_manifest_hsp_rom *manifest, const struct ecc_hw *pka,
	const struct hash_engine *hash, const struct key_manifest_public_key *image_key,
	const struct key_manifest_public_key *secondary_key)
{
	const struct ecc_point_public_key *secondary_ecc = NULL;
	int status;

	code_path_integrity_message_trace (ROM_LOGGING_TRACE_VERIFY_1SP_HEADER);

	if (secondary_key) {
		secondary_ecc = secondary_key->key.ecc;
	}

	status = hsp_fw_1sp_verify_signed_header (image, hash, pka, image_key->key.ecc, secondary_ecc,
		manifest->get_svn (manifest));
	if (status == 0) {
		code_path_integrity_message_trace (ROM_LOGGING_TRACE_READ_1SP);

		status = hsp_fw_1sp_load_image (image, hash);
	}

	if (status != 0) {
		/* If the image failed to load, clean up any initialized instances. */
		hsp_fw_1sp_release (image);
		key_manifest_hsp_rom_release (manifest);
	}

	return status;
}

/**
 * Load and verify a 1SP image from flash into SP memory.
 *
 * @param manifest_flash The flash device that contains the key manifest for the image to load.
 * This will only be used to load the key manifest.
 * @param img_flash An optional flash device to for loading the firmware image header.  If this is
 * null, the same flash device for loading the image data, which is already part of the initialized
 * 1SP image instance will be used to load the header.
 * @param base_addr The starting address in flash for the image.
 * @param pka The PKA engine to use for signature verification.
 * @param hash A hash engine to use for image verification.
 * @param manifest Output for the key manifest of the image on flash.  This must have the base API
 * initialized, but the variable context must be uninitialized.  On successful load, the manifest
 * will be initialized to contain the image manifest information.  On failure, it will remain
 * uninitialized.  This can be a constant structure since only variable state of the instance will
 * change.
 * @param image Output for the image that was loaded.  This must have the base API initialized, but
 * the variable context must be uninitialized.  On successful load, this will be initialized to
 * contain the image metadata.  On failure, it will remain uninitialized.  This can be a constant
 * structure since only the variable state of the instance will change.
 *
 * @return 0 if a valid image was successfully loaded to SP RAM or an error code.
 */
int load_image_1sp_from_flash (const struct flash *manifest_flash, const struct flash *img_flash,
	uint32_t base_addr, const struct ecc_hw *pka, const struct hash_engine *hash,
	const struct key_manifest_hsp_rom *manifest, const struct hsp_fw_1sp *image)
{
	const struct key_manifest_public_key *image_key;
	const struct key_manifest_public_key *secondary_key;
	bool manifest_init = false;
	int status = 0;

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_START_FLASH_LOAD);
	if ((manifest_flash == NULL) || (pka == NULL) || (hash == NULL) || (manifest == NULL) ||
		(image == NULL)) {
		status = LOAD_IMAGE_1SP_INVALID_ARGUMENT;
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_FLASH_MANIFEST);
	if (status == 0) {
		status = manifest->init_state (manifest, manifest_flash, base_addr);

		manifest_init = (status == 0);
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_FLASH_MANIFEST_VALIDATION);
	if (status == 0) {
		status = load_image_1sp_verify_manifest (manifest, hash, &image_key, &secondary_key);
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_FLASH_IMAGE);
	if (status == 0) {
		status = hsp_fw_1sp_init_state (image, img_flash,
			base_addr + manifest->get_total_size (manifest));
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_FLASH_IMAGE_VALIDATION);
	if (status == 0) {
		status = load_image_1sp_verify_image (image, manifest, pka, hash, image_key, secondary_key);
	}
	else if (manifest_init) {
		key_manifest_hsp_rom_release (manifest);
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_FLASH_LOAD_DONE);

	return status;
}

/**
 * Copy and verify a 1SP image from a memory location into SP memory for execution.
 *
 * @param base_addr The location in memory of the image to copy.
 * @param pka The PKA engine to use for signature verification.
 * @param hash A hash engine to use for image verification.
 * @param manifest Output for the key manifest of the image in memory.  This must have the base API
 * initialized, but the variable context must be uninitialized.  On successful load, the manifest
 * will be initialized to contain the image manifest information.  On failure, it will remain
 * uninitialized.  This can be a constant structure since only variable state of the instance will
 * change.
 * @param image Output for the image that was loaded.  This must have the base API initialized, but
 * the variable context must be uninitialized.  On successful load, this will be initialized to
 * contain the image metadata.  On failure, it will remain uninitialized.  This can be a constant
 * structure since only variable state of the instance will change.
 *
 * @return 0 if a valid image was successfully loaded to SP RAM or an error code.
 */
int load_image_1sp_from_memory (const uint8_t *base_addr, const struct ecc_hw *pka,
	const struct hash_engine *hash, const struct key_manifest_hsp_rom *manifest,
	const struct hsp_fw_1sp *image)
{
	const struct key_manifest_public_key *image_key;
	const struct key_manifest_public_key *secondary_key;
	bool manifest_init = false;
	int status = 0;

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_START_MEMORY_LOAD);
	if ((base_addr == NULL) || (pka == NULL) || (hash == NULL) || (manifest == NULL) ||
		(image == NULL)) {
		status = LOAD_IMAGE_1SP_INVALID_ARGUMENT;
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_MEMORY_MANIFEST);
	if (status == 0) {
		status = manifest->init_state_from_memory (manifest, base_addr);

		manifest_init = (status == 0);
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_MEMORY_MANIFEST_VALIDATION);
	if (status == 0) {
		status = load_image_1sp_verify_manifest (manifest, hash, &image_key, &secondary_key);
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_MEMORY_IMAGE);
	if (status == 0) {
		status = hsp_fw_1sp_init_state_from_memory (image,
			base_addr + manifest->get_total_size (manifest));
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_MEMORY_IMAGE_VALIDATION);
	if (status == 0) {
		status = load_image_1sp_verify_image (image, manifest, pka, hash, image_key, secondary_key);
	}
	else if (manifest_init) {
		key_manifest_hsp_rom_release (manifest);
	}

	code_path_integrity_secure_message_no_trace (LOAD_IMAGE_1SP_TRACE_MEMORY_LOAD_DONE);

	return status;
}

/**
 * Load and verify a 1SP firmware image from flash into SP memory.  There can be multiple flash
 * locations and devices that will be inspected for a valid image.  The first valid image found will
 * be loaded.
 *
 * To provide flexibility, the firmware slot definition contains both a manifest and 1SP image
 * instance that will be used to load and validate the image.  The following conditions must be met
 * for each manifest and image instance passed in with a firmware slot:
 * - They must have the base API and static information already initialized.
 * - The variable context must be uninitialized.
 * - On failure, all manifest and image instances will remain uninitialized.
 * - On success, the only the manifest and image instances in the successful firmware slot will be
 * initialized with image metadata.  Other manifest and image instances will remain uninitialized.
 * - The same manifest and image instances can be shared between any number of slots.
 *
 * @param slots A list of flash devices and locations to look for a valid image.  This list will be
 * checked sequentially starting from the first entry.  Image checking will stop at the first list
 * entry that contains a valid image.
 * @param count The number of firmware slots to check.
 * @param pka The PKA engine to use for signature verification.
 * @param hash A hash engine to use for image verification.
 * @param slot Output for the slot index that was used to load the image.  The manifest and 1SP
 * image instances will be initialized for this slot.  The instances on other slots, if they are
 * different, will remain uninitialized.  On failure, this value is not meaningful.
 *
 * @return 0 if an image was successfully loaded from one of the firmware slots or an error code if
 * all slots failed.  If an error is returned, the code represents the error generated when reading
 * the last slot.
 */
int load_image_1sp_from_multiple_flash (const struct load_image_1sp_fw_slot *slots, size_t count,
	const struct ecc_hw *pka, const struct hash_engine *hash, size_t *slot)
{
	int status;

	if ((slots == NULL) || (count == 0) || (pka == NULL) || (hash == NULL) || (slot == NULL)) {
		return LOAD_IMAGE_1SP_INVALID_ARGUMENT;
	}

	*slot = 0;
	do {
		code_path_integrity_message_trace (slots[*slot].trace_msg);

		status = load_image_1sp_from_flash (slots[*slot].manifest_flash, slots[*slot].image_flash,
			slots[*slot].base_addr, pka, hash, slots[*slot].manifest, slots[*slot].image);
		if (status != 0) {
			rom_logging_error (slots[*slot].fail_id, status);

			/* On error, restart the checkpoint chain, but this must be skipped for the last slot
			 * or else calling code will not be able to close the chain for that slot. */
			if (slots[*slot].chkpt && ((*slot) != (count - 1))) {
				code_path_integrity_checkpoint_hand_off (slots[*slot].chkpt->end,
					slots[*slot].chkpt->start);
			}
		}
		else {
			code_path_integrity_random_delay ();
		}
	} while ((status != 0) && (++(*slot) < count));

	return status;
}

/**
 * Update the hardware RoT state to match that of the loaded 1SP image.
 *
 * @param manifest The key manifest for the loaded image, which contains all the information about
 * the image security state.
 * @param hash A hash engine to use for root key hashing.
 * @param tenancy_only Flag to indicate that only the tenancy state of the RoT should be updated.
 * If this is false, all RoT update flows are possible.
 *
 * @return 0 if the hardware RoT matches the security configuration of the 1SP image or an error
 * code.  A successful return doesn't necessarily mean any hardware was update.  It could equally
 * mean that nothing needed to be updated.
 */
int load_image_1sp_update_rot (const struct key_manifest_hsp_rom *manifest,
	const struct hash_engine *hash, bool tenancy_only)
{
	int status = 0;

	if ((manifest == NULL) || (hash == NULL)) {
		return LOAD_IMAGE_1SP_INVALID_ARGUMENT;
	}

	if (!tenancy_only) {
		status = manifest->base.revokes_old_manifest (&manifest->base);
		if (status == 1) {
			code_path_integrity_message_trace (ROM_LOGGING_TRACE_UPDATE_REVOCATION);
			status = 0;
		}

		if (status == 0) {
			/* Always make a call to update revocation data to give the RoT an opportunity to
			 * correct any fuse programming errors.  The manifest call will make sure the right
			 * operation takes place */
			status = manifest->base.update_revocation (&manifest->base);
		}
	}

	if (status == 0) {
		if (!tenancy_only && manifest->is_ownership_transfer (manifest)) {
			code_path_integrity_message_trace (ROM_LOGGING_TRACE_OWNERSHIP_TRANSFER);

			status = manifest->update_root_key (manifest, hash);
			if (status != 0) {
				rom_logging_error (ROM_LOGGING_FAIL_OWNER_TRANSFER, status);
			}
		}
		else if (manifest->is_tenancy_transfer (manifest)) {
			code_path_integrity_message_trace (ROM_LOGGING_TRACE_TENANCY_TRANSFER);

			status = manifest->update_tenancy_counter (manifest);
			if (status != 0) {
				rom_logging_error (ROM_LOGGING_FAIL_TENANT_TRANSFER, status);
			}
		}
	}
	else {
		rom_logging_error (ROM_LOGGING_FAIL_SVN_UPDATE, status);
	}

	return status;
}
