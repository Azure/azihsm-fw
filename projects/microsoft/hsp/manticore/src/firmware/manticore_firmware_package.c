// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "manticore_device_keys.h"
#include "manticore_firmware_package.h"
#include "platform_api.h"
#include "asn1/ecc_der_util.h"
#include "common/buffer_util.h"
#include "flash/flash_util.h"
#include "logging/manticore_logging.h"


/**
 * Marker to identify an SPRT image component.
 */
#define	MANTICORE_FIRMWARE_PACKAGE_SP_IMAGE_MARKER		0x53505254

/**
 * Marker to identify a CP image component.
 */
#define	MANTICORE_FIRMWARE_PACKAGE_CP_IMAGE_MARKER		0x43504657

/**
 * Marker to identify an FP image component.
 */
#define	MANTICORE_FIRMWARE_PACKAGE_FP_IMAGE_MARKER		0x46504657

/**
 * Marker to identify a PCIe image component.
 */
#define	MANTICORE_FIRMWARE_PACKAGE_PCIE_IMAGE_MARKER	0x50434965

/**
 * Marker to identify the IV table.
 */
#define	MANTICORE_FIRMWARE_PACKAGE_IV_MARKER			0x656e6976


/**
 * Inspect and optionally verify a group of components on flash that share the same type.
 *
 * @param fw The firmware package for the image to check.
 * @param start_addr The flash address of the first component.
 * @param hash Hash engine to use for verification.  Set this to null to skip component
 * verification.
 * @param verification Verification interface for component signature verification.  Set this to
 * null to skip component verification.
 * @param marker Identifier for the type of component being verified.
 * @param count The number of components that should be verified.
 * @param end_addr Output for the flash address after the last verified component.
 *
 * @return 0 if all components are valid or an error code.
 */
static int manticore_firmware_package_verify_component_list (
	const struct manticore_firmware_package *fw, uint32_t start_addr,
	const struct hash_engine *hash, const struct signature_verification *verification,
	uint32_t marker, size_t count, uint32_t *end_addr)
{
	struct firmware_component image;
	size_t i = 0;
	int status = 0;

	while ((i < count) && (status == 0)) {
		status = firmware_component_init (&image, fw->flash, start_addr, marker);
		if (status != 0) {
			return status;
		}

		if ((hash != NULL) && (verification != NULL)) {
			status = firmware_component_verification (&image, hash, verification,
				manticore_firmware_descriptor_get_build_version (fw->descriptor), NULL, 0, NULL);
		}

		start_addr = firmware_component_get_image_end (&image);
		firmware_component_release (&image);

		i++;
	}

	*end_addr = start_addr;

	return status;
}

/**
 * Initialize a firmware package handler from data that is stored on flash.
 *
 * @param fw The firmware package handler to initialize.
 * @param descriptor Parser for the package firmware descriptor.  This will get initialized with the
 * package descriptor details.
 * @param flash The flash device that contains the firmware package.
 * @param address The base address for the firmware package on flash.
 * @param hash Hash engine to use for firmware descriptor validation.
 * @param verification Verification interface to use to validate the firmware descriptor.
 * @param ccs Interface to the CCS for IV HMAC verification.
 * @param require_sp Flag indicating if there must be SP images present in the package.
 *
 * @return 0 if the firmware package handler was successfully initialized or an error code.
 */
static int manticore_firmware_package_init_common (struct manticore_firmware_package *fw,
	struct manticore_firmware_descriptor *descriptor, const struct flash *flash, uint32_t address,
	const struct hash_engine *hash, const struct signature_verification *verification,
	const struct ccs_ksu_interface *ccs, bool require_sp)
{
	size_t pkg_data_length;
	size_t pkg_sig_length;
	uint32_t iv_marker;
	size_t iv_length;
	uint8_t iv_hmac[SHA384_HASH_LENGTH];
	size_t i;
	int status;

	if ((fw == NULL) || (ccs == NULL)) {
		return MANTICORE_FW_PACKAGE_INVALID_ARGUMENT;
	}

	memset (fw, 0, sizeof (struct manticore_firmware_package));

	fw->descriptor = descriptor;
	fw->flash = flash;
	fw->base_addr = address;

	status = manticore_firmware_descriptor_init (descriptor, &fw->header, flash, address, hash,
		verification);
	if (status != 0) {
		return status;
	}

	if (require_sp && (manticore_firmware_descriptor_sp_image_count (descriptor) == 0)) {
		status = MANTICORE_FW_PACKAGE_NO_SP_IMAGES;
		goto error_header;
	}

	/* Inspect the images in the package to determine the location of each group of images and to
	 * check for any malformed images. */
	fw->sp_addr = fw->base_addr +
		manticore_firmware_descriptor_get_component_length (fw->descriptor);

	status = manticore_firmware_package_verify_component_list (fw, fw->sp_addr, NULL, NULL,
		MANTICORE_FIRMWARE_PACKAGE_SP_IMAGE_MARKER,
		manticore_firmware_descriptor_sp_image_count (fw->descriptor), &fw->cp_addr);
	if (status != 0) {
		goto error_header;
	}

	status = manticore_firmware_package_verify_component_list (fw, fw->cp_addr, NULL, NULL,
		MANTICORE_FIRMWARE_PACKAGE_CP_IMAGE_MARKER,
		manticore_firmware_descriptor_cp_image_count (fw->descriptor), &fw->fp0_addr);
	if (status != 0) {
		goto error_header;
	}

	status = manticore_firmware_package_verify_component_list (fw, fw->fp0_addr, NULL, NULL,
		MANTICORE_FIRMWARE_PACKAGE_FP_IMAGE_MARKER,
		manticore_firmware_descriptor_fp0_image_count (fw->descriptor), &fw->fp1_addr);
	if (status != 0) {
		goto error_header;
	}

	status = manticore_firmware_package_verify_component_list (fw, fw->fp1_addr, NULL, NULL,
		MANTICORE_FIRMWARE_PACKAGE_FP_IMAGE_MARKER,
		manticore_firmware_descriptor_fp1_image_count (fw->descriptor), &fw->fp2_addr);
	if (status != 0) {
		goto error_header;
	}

	status = manticore_firmware_package_verify_component_list (fw, fw->fp2_addr, NULL, NULL,
		MANTICORE_FIRMWARE_PACKAGE_FP_IMAGE_MARKER,
		manticore_firmware_descriptor_fp2_image_count (fw->descriptor), &fw->pcie_addr);
	if (status != 0) {
		goto error_header;
	}

	status = manticore_firmware_package_verify_component_list (fw, fw->pcie_addr, NULL, NULL,
		MANTICORE_FIRMWARE_PACKAGE_PCIE_IMAGE_MARKER,
		manticore_firmware_descriptor_pcie_image_count (fw->descriptor), &address);
	if (status != 0) {
		goto error_header;
	}

	/* We are assured by the firmware descriptor that the extra images count and signature
	 * information are present in the header. */
	firmware_header_get_signature_info (&fw->header, &pkg_data_length, &pkg_sig_length);
	iv_length =
		firmware_header_get_extra_images (&fw->header) * MANTICORE_FIRMWARE_PACKAGE_IV_LENGTH;

	fw->iv_table = platform_malloc (iv_length);
	if (fw->iv_table == NULL) {
		status = MANTICORE_FW_PACKAGE_NO_MEMORY;
		goto error_header;
	}

	/* The IV table location needs to be determined based on the package size and signature length.
	 * Using the relative address of the last component is not sufficient since there may be other
	 * unknown components in the package. */
	address = fw->base_addr + pkg_data_length + pkg_sig_length;

	status = flash->read (flash, address, (uint8_t*) &iv_marker, sizeof (iv_marker));
	if (status != 0) {
		goto error_iv;
	}

	if (iv_marker != MANTICORE_FIRMWARE_PACKAGE_IV_MARKER) {
		status = MANTICORE_FW_PACKAGE_BAD_IV_MARKER;
		goto error_iv;
	}

	address += sizeof (iv_marker);
	status = flash->read (flash, address, fw->iv_table, iv_length);
	if (status != 0) {
		goto error_iv;
	}

	address += iv_length;
	status = flash->read (flash, address, iv_hmac, sizeof (iv_hmac));
	if (status != 0) {
		goto error_iv;
	}

	/* Check to see if the components have been encrypted by determining if the IV HMAC is not
	 * blank.  If so, verify the IV table contents. */
	fw->encrypted = false;
	i = 0;
	while (!fw->encrypted && (i < sizeof (iv_hmac))) {
		if (iv_hmac[i++] != 0xff) {
			fw->encrypted = true;
		}
	}

	if (fw->encrypted) {
		SP_MSG_384 hmac_actual;

		status = ccs->hmac (ccs, MANTICORE_DEVICE_KEYS_FW_IMAGE_HMAC_KEY, fw->iv_table, iv_length,
			&hmac_actual, NULL);
		if (status != 0) {
			goto error_iv;
		}

		if (buffer_compare (hmac_actual.AsBytes, iv_hmac, SHA384_HASH_LENGTH) != 0) {
			status = MANTICORE_FW_PACKAGE_BAD_IV_TABLE;
			goto error_iv;
		}
	}
	else {
		/* When the image is not encrypted, ensure the IV table is completely blank.  Otherwise, it
		 * means invalid data is stored and encrypting the image later may not work correctly since
		 * writing the IV could fail. */
		i = 0;
		while (i < iv_length) {
			if (fw->iv_table[i++] != 0xff) {
				status = MANTICORE_FW_PACKAGE_IV_TABLE_NOT_BLANK;
				goto error_iv;
			}
		}
	}

	return 0;

error_iv:
	platform_free (fw->iv_table);
error_header:
	firmware_header_release (&fw->header);

	return status;
}

/**
 * Initialize a firmware package handler from data that is stored on flash.
 *
 * @param fw The firmware package handler to initialize.
 * @param descriptor Parser for the package firmware descriptor.  This will get initialized with the
 * package descriptor details.
 * @param flash The flash device that contains the firmware package.
 * @param address The base address for the firmware package on flash.
 * @param hash Hash engine to use for firmware descriptor validation.
 * @param verification Verification interface to use to validate the firmware descriptor.
 * @param ccs Interface to the CCS for IV HMAC verification.
 *
 * @return 0 if the firmware package handler was successfully initialized or an error code.
 */
int manticore_firmware_package_init (struct manticore_firmware_package *fw,
	struct manticore_firmware_descriptor *descriptor, const struct flash *flash, uint32_t address,
	const struct hash_engine *hash, const struct signature_verification *verification,
	const struct ccs_ksu_interface *ccs)
{
	return manticore_firmware_package_init_common (fw, descriptor, flash, address, hash,
		verification, ccs, false);
}

/**
 * Initialize a firmware package handler from data that is stored on flash.  If the package does not
 * contain any SPRT components, it will be treated as invalid.
 *
 * @param fw The firmware package handler to initialize.
 * @param descriptor Parser for the package firmware descriptor.  This will get initialized with the
 * package descriptor details.
 * @param flash The flash device that contains the firmware package.
 * @param address The base address for the firmware package on flash.
 * @param hash Hash engine to use for firmware descriptor validation.
 * @param verification Verification interface to use to validate the firmware descriptor.
 * @param ccs Interface to the CCS for IV HMAC verification.
 *
 * @return 0 if the firmware package handler was successfully initialized or an error code.
 */
int manticore_firmware_package_init_require_sp (struct manticore_firmware_package *fw,
	struct manticore_firmware_descriptor *descriptor, const struct flash *flash, uint32_t address,
	const struct hash_engine *hash, const struct signature_verification *verification,
	const struct ccs_ksu_interface *ccs)
{
	return manticore_firmware_package_init_common (fw, descriptor, flash, address, hash,
		verification, ccs, true);
}

/**
 * Release the resources used by a firmware package handler.
 *
 * @param fw The firmware package handler to release.
 */
void manticore_firmware_package_release (struct manticore_firmware_package *fw)
{
	if (fw) {
		firmware_header_release (&fw->header);
		platform_free (fw->iv_table);
	}
}

/**
 * Get the total length of the firmware package.
 *
 * @param fw The firmware package to query.
 *
 * @return The total package length.
 */
size_t manticore_firmware_package_get_length (const struct manticore_firmware_package *fw)
{
	size_t pkg_data_length;
	size_t pkg_sig_length;
	size_t iv_length;

	if (fw == NULL) {
		return 0;
	}

	/* These calls will not fail in this context. */
	firmware_header_get_signature_info (&fw->header, &pkg_data_length, &pkg_sig_length);
	iv_length =
		firmware_header_get_extra_images (&fw->header) * MANTICORE_FIRMWARE_PACKAGE_IV_LENGTH;

	/* The total size is the signed package data, package signature, IV table, IV marker, and
	 * IV HMAC.*/
	return pkg_data_length + pkg_sig_length + iv_length + sizeof (uint32_t) + SHA384_HASH_LENGTH;
}

/**
 * Get the firmware header for the firmware package.
 *
 * @param fw The firmware package to query.
 *
 * @return The package firmware header or null if there is an error.
 */
const struct firmware_header* manticore_firmware_package_get_firmware_header (
	const struct manticore_firmware_package *fw)
{
	if (fw) {
		return &fw->header;
	}
	else {
		return NULL;
	}
}

/**
 * Verify the signature of the entire firmware package on flash.
 *
 * @param fw The firmware package to verify.
 * @param hash Hash engine to use for validation.
 * @param verification Verification interface for package signature verification.
 *
 * @return 0 if the firmware package on flash is valid or an error code.
 */
int manticore_firmware_package_verify_package_on_flash (const struct manticore_firmware_package *fw,
	const struct hash_engine *hash, const struct signature_verification *verification)
{
	size_t pkg_data_length;
	size_t pkg_sig_length;
	uint8_t signature[ECC_DER_P384_ECDSA_MAX_LENGTH] = {0};	// Manticore images use P-384 signatures.
	int status;

	if ((fw == NULL) || (hash == NULL) || (verification == NULL)) {
		return MANTICORE_FW_PACKAGE_INVALID_ARGUMENT;
	}

	/* This call will not fail in this context. */
	firmware_header_get_signature_info (&fw->header, &pkg_data_length, &pkg_sig_length);
	if (pkg_sig_length > sizeof (signature)) {
		return MANTICORE_FW_PACKAGE_SIG_TOO_LONG;
	}

	status = fw->flash->read (fw->flash, fw->base_addr + pkg_data_length, signature,
		pkg_sig_length);
	if (status != 0) {
		return status;
	}

	status = flash_contents_verification (fw->flash, fw->base_addr, pkg_data_length, hash,
		HASH_TYPE_SHA384, verification, signature, pkg_sig_length, NULL, 0);

	buffer_zeroize (signature, sizeof (signature));

	return status;
}

/**
 * Verify the signatures of each component within the firmware package on flash.  Build versions are
 * also checked for consistency.
 *
 * The firmware descriptor is not verified since that was already verified during initialization.
 *
 * @param fw The firmware package to verify.
 * @param hash Hash engine to use for verification.
 * @param verification Verification interface for component signature verification.
 *
 * @return 0 if the firmware package components are valid or an error code.
 */
int manticore_firmware_package_verify_components_on_flash (
	const struct manticore_firmware_package *fw, const struct hash_engine *hash,
	const struct signature_verification *verification)
{
	uint32_t start_addr;
	int status;

	if ((fw == NULL) || (hash == NULL) || (verification == NULL)) {
		return MANTICORE_FW_PACKAGE_INVALID_ARGUMENT;
	}

	start_addr = fw->base_addr +
		manticore_firmware_descriptor_get_component_length (fw->descriptor);

	/* Verify the SPRT image components. */
	status = manticore_firmware_package_verify_component_list (fw, start_addr, hash, verification,
		MANTICORE_FIRMWARE_PACKAGE_SP_IMAGE_MARKER,
		manticore_firmware_descriptor_sp_image_count (fw->descriptor), &start_addr);
	if (status != 0) {
		return status;
	}

	/* Verify the CP image components. */
	status = manticore_firmware_package_verify_component_list (fw, start_addr, hash, verification,
		MANTICORE_FIRMWARE_PACKAGE_CP_IMAGE_MARKER,
		manticore_firmware_descriptor_cp_image_count (fw->descriptor), &start_addr);
	if (status != 0) {
		return status;
	}

	/* Verify the FP image components. */
	status = manticore_firmware_package_verify_component_list (fw, start_addr, hash, verification,
		MANTICORE_FIRMWARE_PACKAGE_FP_IMAGE_MARKER,
		manticore_firmware_descriptor_fp_image_count (fw->descriptor), &start_addr);
	if (status != 0) {
		return status;
	}

	/* Verify the PCIe image components. */
	status = manticore_firmware_package_verify_component_list (fw, start_addr, hash, verification,
		MANTICORE_FIRMWARE_PACKAGE_PCIE_IMAGE_MARKER,
		manticore_firmware_descriptor_pcie_image_count (fw->descriptor), &start_addr);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Load a set of images from flash into memory.  Verify that the data loaded into memory is valid.
 *
 * All loaded images must be the same type of component.
 *
 * @param fw The firmware package to load the images from.
 * @param loader Handler for loading the data into memory.
 * @param start_addr The flash address of the first component.
 * @param hash Hash engine for validating the loaded images.
 * @param verification Verification interface for image signature verification.
 * @param marker Identifier for the type of component being loaded.
 * @param log_id ID for the firmware image to use for log error messages.
 * @param count The number of components that should be loaded.
 * @param iv_offset Offset into the IV table for the images being loaded.
 * @param digests Optional output to a list of SHA2-384 digests for the images.  Each image
 * will have a separate digest in the list.  The memory for these digests will be dynamically
 * allocated and must be freed by the caller.
 *
 * @return 0 if all images were loaded and validated successfully or an error code.
 */
static int manticore_firmware_package_load_component_list (
	const struct manticore_firmware_package *fw, const struct firmware_loader *loader,
	uint32_t start_addr, const struct hash_engine *hash,
	const struct signature_verification *verification, uint32_t marker, uint8_t log_id,
	size_t count, int iv_offset, uint8_t **digests)
{
	struct firmware_component image;
	uint8_t *hash_out = NULL;
	const uint8_t *iv = NULL;
	size_t iv_length = 0;
	size_t i = 0;
	int status = 0;

	if (digests != NULL) {
		*digests = platform_calloc (count, SHA384_HASH_LENGTH);
		if (*digests == NULL) {
			return MANTICORE_FW_PACKAGE_NO_MEMORY;
		}
	}

	while (i < count) {
		status = firmware_component_init (&image, fw->flash, start_addr, marker);
		if (status != 0) {
			break;
		}

		if (digests != NULL) {
			hash_out = &(*digests)[SHA384_HASH_LENGTH * i];
		}

		if (fw->encrypted) {
			iv = &fw->iv_table[(i + iv_offset) * MANTICORE_FIRMWARE_PACKAGE_IV_LENGTH];
			iv_length = MANTICORE_FIRMWARE_PACKAGE_IV_LENGTH;
		}

		status = firmware_component_load_to_memory_and_verify (&image, loader, iv, iv_length, hash,
			verification, manticore_firmware_descriptor_get_build_version (fw->descriptor),
			hash_out, SHA384_HASH_LENGTH, NULL, NULL);

		start_addr = firmware_component_get_image_end (&image);
		firmware_component_release (&image);

		if (status != 0) {
			break;
		}

		i++;
	}

	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_COMPONENT_LOAD_ERROR, log_id, i);

		if (digests != NULL) {
			platform_free (*digests);
			*digests = NULL;
		}
	}

	return status;
}

/**
 * Load all SPRT images from flash into memory.  Verify that the data loaded into memory is valid.
 *
 * @param fw The firmware package to load the SPRT images from.
 * @param loader Handler for loading the data into SP memory.
 * @param hash Hash engine for validating the loaded images.
 * @param verification Verification interface for image signature verification.
 * @param digests Optional output to a list of SHA2-384 digests for the SPRT images.  Each image
 * will have a separate digest in the list.  The memory for these digests will be dynamically
 * allocated and must be freed by the caller.
 * @param img_count Optional output for the number of images that were loaded.  This is required if
 * the digest output is provided.
 *
 * @return 0 if all SPRT images were loaded and validated successfully or an error code.
 */
int manticore_firmware_package_load_sp_components (const struct manticore_firmware_package *fw,
	const struct firmware_loader *loader, const struct hash_engine *hash,
	const struct signature_verification *verification, uint8_t **digests, size_t *img_count)
{
	uint8_t sp_count;

	if ((fw == NULL) || (loader == NULL) || (hash == NULL) || (verification == NULL) ||
		((digests != NULL) && (img_count == NULL))) {
		return MANTICORE_FW_PACKAGE_INVALID_ARGUMENT;
	}

	sp_count = manticore_firmware_descriptor_sp_image_count (fw->descriptor);
	if (sp_count == 0) {
		return MANTICORE_FW_PACKAGE_NO_SP_IMAGES;
	}

	if (img_count != NULL) {
		*img_count = sp_count;
	}

	return manticore_firmware_package_load_component_list (fw, loader, fw->sp_addr, hash,
		verification, MANTICORE_FIRMWARE_PACKAGE_SP_IMAGE_MARKER, MANTICORE_LOGGING_SP_FW_COMPONENT,
		sp_count, 0, digests);
}

/**
 * Load all CP images from flash into memory.  Verify that the data loaded into memory is valid.
 *
 * @param fw The firmware package to load the CP images from.
 * @param loader Handler for loading the data into CP memory.
 * @param hash Hash engine for validating the loaded images.
 * @param verification Verification interface for image signature verification.
 * @param digests Optional output to a list of SHA2-384 digests for the CP images.  Each image
 * will have a separate digest in the list.  The memory for these digests will be dynamically
 * allocated and must be freed by the caller.
 * @param img_count Optional output for the number of images that were loaded.  This is required if
 * the digest output is provided.
 *
 * @return 0 if all CP images were loaded and validated successfully or an error code.
 */
int manticore_firmware_package_load_cp_components (const struct manticore_firmware_package *fw,
	const struct firmware_loader *loader, const struct hash_engine *hash,
	const struct signature_verification *verification, uint8_t **digests, size_t *img_count)
{
	uint8_t cp_count;

	if ((fw == NULL) || (loader == NULL) || (hash == NULL) || (verification == NULL) ||
		((digests != NULL) && (img_count == NULL))) {
		return MANTICORE_FW_PACKAGE_INVALID_ARGUMENT;
	}

	cp_count = manticore_firmware_descriptor_cp_image_count (fw->descriptor);
	if (cp_count == 0) {
		return MANTICORE_FW_PACKAGE_NO_CP_IMAGES;
	}

	if (img_count != NULL) {
		*img_count = cp_count;
	}

	return manticore_firmware_package_load_component_list (fw, loader, fw->cp_addr, hash,
		verification, MANTICORE_FIRMWARE_PACKAGE_CP_IMAGE_MARKER, MANTICORE_LOGGING_CP_FW_COMPONENT,
		cp_count, manticore_firmware_descriptor_sp_image_count (fw->descriptor), digests);
}

/**
 * Load all FP images for core 0 from flash into memory.  Verify that the data loaded into memory is
 * valid.
 *
 * @param fw The firmware package to load the FP core 0 images from.
 * @param loader Handler for loading the data into FP core 0 memory.
 * @param hash Hash engine for validating the loaded images.
 * @param verification Verification interface for image signature verification.
 * @param digests Optional output to a list of SHA2-384 digests for the FP core 0 images.  Each
 * image will have a separate digest in the list.  The memory for these digests will be dynamically
 * allocated and must be freed by the caller.
 * @param img_count Optional output for the number of images that were loaded.  This is required if
 * the digest output is provided.
 *
 * @return 0 if all FP core 0 images were loaded and validated successfully or an error code.
 */
int manticore_firmware_package_load_fp0_components (const struct manticore_firmware_package *fw,
	const struct firmware_loader *loader, const struct hash_engine *hash,
	const struct signature_verification *verification, uint8_t **digests, size_t *img_count)
{
	uint8_t fp0_count;
	int iv_offset;

	if ((fw == NULL) || (loader == NULL) || (hash == NULL) || (verification == NULL) ||
		((digests != NULL) && (img_count == NULL))) {
		return MANTICORE_FW_PACKAGE_INVALID_ARGUMENT;
	}

	fp0_count = manticore_firmware_descriptor_fp0_image_count (fw->descriptor);
	if (fp0_count == 0) {
		return MANTICORE_FW_PACKAGE_NO_FP0_IMAGES;
	}

	if (img_count != NULL) {
		*img_count = fp0_count;
	}

	iv_offset = manticore_firmware_descriptor_sp_image_count (fw->descriptor) +
		manticore_firmware_descriptor_cp_image_count (fw->descriptor);

	return manticore_firmware_package_load_component_list (fw, loader, fw->fp0_addr, hash,
		verification, MANTICORE_FIRMWARE_PACKAGE_FP_IMAGE_MARKER,
		MANTICORE_LOGGING_FP0_FW_COMPONENT, fp0_count, iv_offset, digests);
}

/**
 * Load all FP images for core 1 from flash into memory.  Verify that the data loaded into memory is
 * valid.
 *
 * @param fw The firmware package to load the FP core 1 images from.
 * @param loader Handler for loading the data into FP core 1 memory.
 * @param hash Hash engine for validating the loaded images.
 * @param verification Verification interface for image signature verification.
 * @param digests Optional output to a list of SHA2-384 digests for the FP core 1 images.  Each
 * image will have a separate digest in the list.  The memory for these digests will be dynamically
 * allocated and must be freed by the caller.
 * @param img_count Optional output for the number of images that were loaded.  This is required if
 * the digest output is provided.
 *
 * @return 0 if all FP core 1 images were loaded and validated successfully or an error code.
 */
int manticore_firmware_package_load_fp1_components (const struct manticore_firmware_package *fw,
	const struct firmware_loader *loader, const struct hash_engine *hash,
	const struct signature_verification *verification, uint8_t **digests, size_t *img_count)
{
	uint8_t fp1_count;
	int iv_offset;

	if ((fw == NULL) || (loader == NULL) || (hash == NULL) || (verification == NULL) ||
		((digests != NULL) && (img_count == NULL))) {
		return MANTICORE_FW_PACKAGE_INVALID_ARGUMENT;
	}

	fp1_count = manticore_firmware_descriptor_fp1_image_count (fw->descriptor);
	if (fp1_count == 0) {
		return MANTICORE_FW_PACKAGE_NO_FP1_IMAGES;
	}

	if (img_count != NULL) {
		*img_count = fp1_count;
	}

	iv_offset = manticore_firmware_descriptor_sp_image_count (fw->descriptor) +
		manticore_firmware_descriptor_cp_image_count (fw->descriptor) +
		manticore_firmware_descriptor_fp0_image_count (fw->descriptor);

	return manticore_firmware_package_load_component_list (fw, loader, fw->fp1_addr, hash,
		verification, MANTICORE_FIRMWARE_PACKAGE_FP_IMAGE_MARKER,
		MANTICORE_LOGGING_FP1_FW_COMPONENT, fp1_count, iv_offset, digests);
}

/**
 * Load all FP images for core 2 from flash into memory.  Verify that the data loaded into memory is
 * valid.
 *
 * @param fw The firmware package to load the FP core 2 images from.
 * @param loader Handler for loading the data into FP core 2 memory.
 * @param hash Hash engine for validating the loaded images.
 * @param verification Verification interface for image signature verification.
 * @param digests Optional output to a list of SHA2-384 digests for the FP core 2 images.  Each
 * image will have a separate digest in the list.  The memory for these digests will be dynamically
 * allocated and must be freed by the caller.
 * @param img_count Optional output for the number of images that were loaded.  This is required if
 * the digest output is provided.
 *
 * @return 0 if all FP core 2 images were loaded and validated successfully or an error code.
 */
int manticore_firmware_package_load_fp2_components (const struct manticore_firmware_package *fw,
	const struct firmware_loader *loader, const struct hash_engine *hash,
	const struct signature_verification *verification, uint8_t **digests, size_t *img_count)
{
	uint8_t fp2_count;
	int iv_offset;

	if ((fw == NULL) || (loader == NULL) || (hash == NULL) || (verification == NULL) ||
		((digests != NULL) && (img_count == NULL))) {
		return MANTICORE_FW_PACKAGE_INVALID_ARGUMENT;
	}

	fp2_count = manticore_firmware_descriptor_fp2_image_count (fw->descriptor);
	if (fp2_count == 0) {
		return MANTICORE_FW_PACKAGE_NO_FP2_IMAGES;
	}

	if (img_count != NULL) {
		*img_count = fp2_count;
	}

	iv_offset = manticore_firmware_descriptor_sp_image_count (fw->descriptor) +
		manticore_firmware_descriptor_cp_image_count (fw->descriptor) +
		manticore_firmware_descriptor_fp0_image_count (fw->descriptor) +
		manticore_firmware_descriptor_fp1_image_count (fw->descriptor);

	return manticore_firmware_package_load_component_list (fw, loader, fw->fp2_addr, hash,
		verification, MANTICORE_FIRMWARE_PACKAGE_FP_IMAGE_MARKER,
		MANTICORE_LOGGING_FP2_FW_COMPONENT, fp2_count, iv_offset, digests);
}

/**
 * Load all PCIe PHY images from flash into memory.  Verify that the data loaded into memory is
 * valid.
 *
 * @param fw The firmware package to load the PCIe PHY images from.
 * @param loader Handler for loading the data into PCIe PHY memory.
 * @param hash Hash engine for validating the loaded images.
 * @param verification Verification interface for image signature verification.
 * @param digests Optional output to a list of SHA2-384 digests for the PCIe PHY images.  Each image
 * will have a separate digest in the list.  The memory for these digests will be dynamically
 * allocated and must be freed by the caller.
 * @param img_count Optional output for the number of images that were loaded.  This is required if
 * the digest output is provided.
 *
 * @return 0 if all PCIe PHY images were loaded and validated successfully or an error code.
 */
int manticore_firmware_package_load_pcie_components (const struct manticore_firmware_package *fw,
	const struct firmware_loader *loader, const struct hash_engine *hash,
	const struct signature_verification *verification, uint8_t **digests, size_t *img_count)
{
	uint8_t pcie_count;
	int iv_offset;

	if ((fw == NULL) || (loader == NULL) || (hash == NULL) || (verification == NULL) ||
		((digests != NULL) && (img_count == NULL))) {
		return MANTICORE_FW_PACKAGE_INVALID_ARGUMENT;
	}

	pcie_count = manticore_firmware_descriptor_pcie_image_count (fw->descriptor);
	if (pcie_count == 0) {
		return MANTICORE_FW_PACKAGE_NO_PCIE_IMAGES;
	}

	if (img_count != NULL) {
		*img_count = pcie_count;
	}

	iv_offset = manticore_firmware_descriptor_sp_image_count (fw->descriptor) +
		manticore_firmware_descriptor_cp_image_count (fw->descriptor) +
		manticore_firmware_descriptor_fp_image_count (fw->descriptor);

	return manticore_firmware_package_load_component_list (fw, loader, fw->pcie_addr, hash,
		verification, MANTICORE_FIRMWARE_PACKAGE_PCIE_IMAGE_MARKER,
		MANTICORE_LOGGING_PHY_FW_COMPONENT, pcie_count, iv_offset, digests);
}
