// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "manticore_firmware_descriptor.h"


/**
 * Marker to identify a firmware descriptor component.
 */
#define	MANTICORE_FIRMWARE_DESCRIPTOR_MARKER			0x4d465744

/**
 * Defines the minimum allowed format for a firmware header.
 */
#define	MANTICORE_FIRMWARE_DESCRIPTOR_MIN_FW_HEADER		3


/**
 * Load a firmware descriptor from flash and verify it.  The firmware header that is prepended to
 * the descriptor will also be loaded from flash.
 *
 * An initialized descriptor does not need to be released and the memory loaded with the descriptor
 * data can be used again without being reinitialized, as long as the data contents are known to not
 * have been changed.
 *
 * @param fw_descriptor The firmware descriptor to initialize.
 * @param fw_header The firmware header to initialize with the firmware descriptor.  This must be
 * uninitialized.  If there is a failure, this will remain uninitialized.
 * @param flash The flash that contains the firmware descriptor data.
 * @param address Base address of the firmware package.  This will be the address of the firmware
 * header.
 * @param hash Hash engine to use for verification of the firmware descriptor.
 * @param verification Verification interface for the firmware component.
 *
 * @return 0 if the firmware descriptor was loaded and successfully verified or an error code.
 */
int manticore_firmware_descriptor_init (struct manticore_firmware_descriptor *fw_descriptor,
	struct firmware_header *fw_header, const struct flash *flash, uint32_t address,
	const struct hash_engine *hash, const struct signature_verification *verification)
{
	struct firmware_component descriptor;
	const uint8_t *build_ver;
	size_t pkg_data_length;
	size_t pkg_sig_length;
	int extra_imgs;
	int status;

	if ((fw_descriptor == NULL) || (fw_header == NULL) || (flash == NULL) || (hash == NULL) ||
		(verification == NULL)) {
		return MANTICORE_FW_DESCRIPTOR_INVALID_ARGUMENT;
	}

	memset (fw_descriptor, 0, sizeof (struct manticore_firmware_descriptor));

	status = firmware_header_init (fw_header, flash, address);
	if (status != 0) {
		return status;
	}

	/* Make sure the firmware header contains information for an overall package signature. */
	status = firmware_header_get_signature_info (fw_header, &pkg_data_length, &pkg_sig_length);
	if (status != 0) {
		/* This will only fail if the FW header format does not contain the information. */
		status = MANTICORE_FW_DESCRIPTOR_OLD_FW_HEADER;
		goto exit_no_component;
	}
	else if ((pkg_data_length == 0) || (pkg_sig_length == 0)) {
		status = MANTICORE_FW_DESCRIPTOR_NO_PKG_SIGNATURE;
		goto exit_no_component;
	}

	status = firmware_component_init_with_header (&descriptor, flash, address,
		MANTICORE_FIRMWARE_DESCRIPTOR_MARKER, image_header_get_length (&fw_header->base));
	if (status != 0) {
		goto exit_no_component;
	}

	fw_descriptor->data_length = firmware_component_get_length (&descriptor);
	if (fw_descriptor->data_length > MANTICORE_FIRMWARE_DESCRIPTOR_MAX_LENGTH) {
		status = MANTICORE_FW_DESCRIPTOR_TOO_LARGE;
		goto exit;
	}
	else if (fw_descriptor->data_length < sizeof (struct manticore_firmware_descriptor_data_v0)) {
		status = MANTICORE_FW_DESCRIPTOR_TOO_SHORT;
		goto exit;
	}
	else if (fw_descriptor->data_length > sizeof (struct manticore_firmware_descriptor_data)) {
		/* There is extra data in the descriptor. */
	}
	else if (fw_descriptor->data_length == sizeof (struct manticore_firmware_descriptor_data)) {
		/* Descriptor format 3. */
	}
	else if (fw_descriptor->data_length == sizeof (struct manticore_firmware_descriptor_data_v2)) {
		/* Descriptor format 2. */
	}
	else if (fw_descriptor->data_length == sizeof (struct manticore_firmware_descriptor_data_v1)) {
		/* Descriptor format 1. */
	}
	else if (fw_descriptor->data_length == sizeof (struct manticore_firmware_descriptor_data_v0)) {
		/* Descriptor format 0. */
	}
	else {
		status = MANTICORE_FW_DESCRIPTOR_BAD_FORMAT;
		goto exit;
	}

	build_ver = firmware_component_get_build_version (&descriptor);
	if (build_ver == NULL) {
		status = MANTICORE_FW_DESCRIPTOR_NO_BUILD_VERSION;
		goto exit;
	}

	/* Make a copy of the build version for the firmware package and the total component length of
	 * the descriptor since these will get lost when the component header is released.  Also keep a
	 * copy of the recovery revision information from the firmware header so that is available in
	 * any cached copy of the descriptor. */
	memcpy (fw_descriptor->build_ver, build_ver, sizeof (fw_descriptor->build_ver));
	fw_descriptor->length = firmware_component_get_total_length (&descriptor);
	firmware_header_get_recovery_revision (fw_header, &fw_descriptor->recovery_rev);
	firmware_header_get_earliest_allowed_revision (fw_header, &fw_descriptor->allowed_rev);

	status = firmware_component_load_and_verify_with_header (&descriptor, fw_descriptor->max_data,
		sizeof (fw_descriptor->max_data), &fw_header->base, hash, verification, NULL, NULL, 0, NULL,
		NULL);
	if (status != 0) {
		goto exit;
	}

	/* Check that the firmware header and firmware descriptor both report the same number of images
	 * contained within the package.  This check can only be performed if there is not extra,
	 * unknown information in the descriptor data. */
	switch (fw_descriptor->data_length) {
		case sizeof (struct manticore_firmware_descriptor_data_v0):
			extra_imgs = fw_descriptor->data.sp_count + fw_descriptor->data.cp_count +
				fw_descriptor->data.fp0_count + fw_descriptor->data.fp1_count +
				fw_descriptor->data.fp2_count;
			break;

		case sizeof (struct manticore_firmware_descriptor_data_v1):
		case sizeof (struct manticore_firmware_descriptor_data_v2):
		case sizeof (struct manticore_firmware_descriptor_data):
			extra_imgs = fw_descriptor->data.sp_count + fw_descriptor->data.cp_count +
				fw_descriptor->data.fp0_count + fw_descriptor->data.fp1_count +
				fw_descriptor->data.fp2_count + fw_descriptor->data.pcie_count;
			break;

		default:
			extra_imgs = -1;
			break;
	}

	if ((extra_imgs >= 0) && (firmware_header_get_extra_images (fw_header) != extra_imgs)) {
		status = MANTICORE_FW_DESCRIPTOR_INCONSISTENT;
	}

exit:
	firmware_component_release (&descriptor);

exit_no_component:
	if (status != 0) {
		firmware_header_release (fw_header);
	}

	return status;
}

/**
 * Get the total length of the firmware component that contains the firmware descriptor.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The total descriptor length or 0 if the descriptor is null.
 */
size_t manticore_firmware_descriptor_get_component_length (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->length;
	}
	else {
		return 0;
	}
}

/**
 * Get the length of only the firmware descriptor data that is contained in the component.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The descriptor data length or 0 if the descriptor is null.
 */
size_t manticore_firmware_descriptor_get_data_length (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->data_length;
	}
	else {
		return 0;
	}
}

/**
 * Get the SVN value reported by the firmware descriptor.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The SVN reported in the descriptor or 0 if the descriptor is null.
 */
uint64_t manticore_firmware_descriptor_get_svn (const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->data.svn;
	}
	else {
		return 0;
	}
}

/**
 * Get the recovery revision identifier for the firmware package.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The firmware package recovery revision or 0 if the descriptor is null.
 */
int manticore_firmware_descriptor_get_recovery_revision (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->recovery_rev;
	}
	else {
		return 0;
	}
}

/**
 * Get the earliest allowed recovery revision for the firmware package.  This functions the same as
 * an SVN, but lacks any HW backing.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The firmware package allowed recovery revision or 0 if the descriptor is null.
 */
int manticore_firmware_descriptor_get_earliest_allowed_revision (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->allowed_rev;
	}
	else {
		return 0;
	}
}

/**
 * Get the build version for the firmware descriptor.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The build version for the descriptor or null if the descriptor is null.
 */
const uint8_t* manticore_firmware_descriptor_get_build_version (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->build_ver;
	}
	else {
		return NULL;
	}
}

/**
 * Get the number of SP images contained within the complete firmware package.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The number of loadable SP images or 0 if the descriptor is null.
 */
uint8_t manticore_firmware_descriptor_sp_image_count (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->data.sp_count;
	}
	else {
		return 0;
	}
}

/**
 * Get the reset vector for the SP run-time image.  This is the address that will be executed after
 * all boot functions have completed.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The SPRT reset vector or 0 if the descriptor is null.
 */
uint32_t manticore_firmware_descriptor_sp_reset_vector (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->data.reset_vector;
	}
	else {
		return 0;
	}
}

/**
 * Get the number of CP images contained within the complete firmware package.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The number of loadable CP images or 0 if the descriptor is null.
 */
uint8_t manticore_firmware_descriptor_cp_image_count (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->data.cp_count;
	}
	else {
		return 0;
	}
}

/**
 * Get the number of images for all FP cores contained within the complete firmware package.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The number of loadable FP images or 0 if the descriptor is null.
 */
uint16_t manticore_firmware_descriptor_fp_image_count (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->data.fp0_count + fw->data.fp1_count + fw->data.fp2_count;
	}
	else {
		return 0;
	}
}

/**
 * Get the number of images for FP core 0 contained within the complete firmware package.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The number of loadable FP0 images or 0 if the descriptor is null.
 */
uint8_t manticore_firmware_descriptor_fp0_image_count (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->data.fp0_count;
	}
	else {
		return 0;
	}
}

/**
 * Get the number of images for FP core 1 contained within the complete firmware package.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The number of loadable FP1 images or 0 if the descriptor is null.
 */
uint8_t manticore_firmware_descriptor_fp1_image_count (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->data.fp1_count;
	}
	else {
		return 0;
	}
}

/**
 * Get the number of images for FP core 2 contained within the complete firmware package.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The number of loadable FP2 images or 0 if the descriptor is null.
 */
uint8_t manticore_firmware_descriptor_fp2_image_count (
	const struct manticore_firmware_descriptor *fw)
{
	if (fw) {
		return fw->data.fp2_count;
	}
	else {
		return 0;
	}
}

/**
 * Get the number of images for the PCIe PHY controller contained within the complete firmware
 * package.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The number of loadable PCIe PHY images.  If the descriptor is null or format 0, 0 is
 * returned.
 */
uint8_t manticore_firmware_descriptor_pcie_image_count (
	const struct manticore_firmware_descriptor *fw)
{
	/* There is no need to check the descriptor length before reading this value since the structure
	 * is zeroed out during init. */
	if (fw) {
		return fw->data.pcie_count;
	}
	else {
		return 0;
	}
}

/**
 * Get the image compatibility version number used to determine if impactless firmware updates can
 * be performed.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The image compatibility version number reported in the descriptor.  If the descriptor is
 * null or format 0, 0 is returned.
 */
uint16_t manticore_firmware_descriptor_image_compatibility_version (
	const struct manticore_firmware_descriptor *fw)
{
	/* Just like the pcie_count case, no length check is necessary due to the data being zeroed out.
	 * Compatibility versioning starts at 1, so a 0 will always be incompatible with anything that
	 * expects a value here. */
	if (fw) {
		return fw->data.compat_version;
	}
	else {
		return 0;
	}
}

/**
 * Get the indication used to determine if the firmware image has been certified by FIPS.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The FIPS certification indicator reported in the descriptor.  If the descriptor is null
 * or not at least format 2, 0 is returned.
 */
uint8_t manticore_firmware_descriptor_fips_certified (
	const struct manticore_firmware_descriptor *fw)
{
	/* No need to check the data length here since older descriptors will have this region of memory
	 * zeroed out during initialization.  Anything that uses an older descriptor format cannot be
	 * FIPS certified. */
	if (fw) {
		return fw->data.fips_certified;
	}
	else {
		return 0;
	}
}

/**
 * Get the indication used to determine if BKS derivations should be different between FIPS and
 * Non-FIPS operation.
 *
 * @param fw The firmware descriptor to query.
 *
 * @return The BKS FIPS isolation indicator reported in the descriptor.  If the descriptor is null
 * or not at least format 3, 0 is returned.
 */
uint8_t manticore_firmware_descriptor_bks_fips_isolation (
	const struct manticore_firmware_descriptor *fw)
{
	/* No need to check the data length since older descriptors will have this region of memory
	 * zeroed out during initialization.  Images with older descriptors don't use BKS. */
	if (fw) {
		return fw->data.bks_isolation;
	}
	else {
		return 0;
	}
}
