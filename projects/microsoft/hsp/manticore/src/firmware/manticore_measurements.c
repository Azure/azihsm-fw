// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "manticore_measurements.h"
#include "asn1/ecc_der_util.h"
#include "common/array_size.h"
#include "common/buffer_util.h"


/**
 * Initial PCR value for an empty PCR.  The first byte must be set to the desired PCR number.
 */
static const uint8_t MANTICORE_MEASUREMENTS_INIT_PCR_VALUE[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/**
 * Measure the public key used to verify the firmware package.
 *
 * @param fw_manifest The firmware key manifest with the key to measure.
 * @param hash Hash engine to use to generate the measurement.
 * @param fw_pkg_public_key Output for the public key event data.
 *
 * @return 0 if the measurement was generated successfully or an error code.
 */
static int manticore_measurements_measure_fw_pkg_key (
	const struct key_manifest_hsp_firmware *fw_manifest, const struct hash_engine *hash,
	struct boot_measurements_public_key *fw_pkg_public_key)
{
	const struct key_manifest_public_key *key;
	int status;

	key = fw_manifest->base.get_app_key (&fw_manifest->base);
	if (key == NULL) {
		return MANTICORE_MEASUREMENTS_NO_FW_KEY;
	}

	status = ecc_der_decode_public_key (key->key.ecc_der_ref.der, key->key.ecc_der_ref.length,
		fw_pkg_public_key->data.key.Parts.X.AsBytes, fw_pkg_public_key->data.key.Parts.Y.AsBytes,
		ECC_KEY_LENGTH_384);
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return hash->calculate_sha384 (hash, (uint8_t*) &fw_pkg_public_key->data,
		sizeof (fw_pkg_public_key->data), fw_pkg_public_key->digest.AsBytes, SP_MSG_384_SIZE);
}

/**
 * Generate a measurement log for the loaded SP firmware.
 *
 * @param log The log that will be populated with measurements.
 * @param hash Hash engine to use for calculating event digests.
 * @param policy The security policy that has been applied to this boot context.
 * @param fw_manifest The firmware key manifest used for firmware package verification.
 * @param fw_descriptor The firmware descriptor in the loaded firmware package.
 * @param sp_digest Digest of the complete SPRT firmware image.
 *
 * @return 0 if the SP measurement log was generated successfully or an error code.
 */
int manticore_measurements_generate_sp_log (struct manticore_measurements_log_sp *log,
	const struct hash_engine *hash, const struct security_policy_hsp_manticore_data *policy,
	const struct key_manifest_hsp_firmware *fw_manifest,
	const struct manticore_firmware_descriptor *fw_descriptor, const SP_MSG_384 *sp_digest)
{
	int status;

	if ((log == NULL) || (hash == NULL) || (policy == NULL) || (fw_manifest == NULL) ||
		(fw_descriptor == NULL) || (sp_digest == NULL)) {
		return MANTICORE_MEASUREMENTS_INVALID_ARGUMENT;
	}

	memset (log, 0, sizeof (struct manticore_measurements_log_sp));

	/* Measure the security policy applied when booting the device. */
	log->security_policy.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_SECURITY_POLICY;
	memcpy (&log->security_policy.data.policy, policy, sizeof (log->security_policy.data.policy));

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->security_policy.data,
		sizeof (log->security_policy.data), log->security_policy.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the digest for the Firmware Key Manifest, which represents the list of keys used by
	 * device for various operations.  The assumption is that the digest in the header matches the
	 * contents of the payload.  If it didn't, the manifest would have already been rejected. */
	log->fw_key_manifest.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_FW_KEY_MANIFEST;
	memcpy (log->fw_key_manifest.data.digest.AsBytes, fw_manifest->keys->header_signed.digest,
		SP_MSG_384_SIZE);

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fw_key_manifest.data,
		sizeof (log->fw_key_manifest.data), log->fw_key_manifest.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the SVN for the Firmware Key Manifest. */
	log->fw_key_manifest_svn.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_FW_KEY_MANIFEST_SVN;
	log->fw_key_manifest_svn.data.svn = fw_manifest->keys->header_signed.svn;

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fw_key_manifest_svn.data,
		sizeof (log->fw_key_manifest_svn.data), log->fw_key_manifest_svn.digest.AsBytes,
		SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the public key from the Firmware Key Manifest that was used to verify the Firmware
	 * Package. */
	log->fw_pkg_public_key.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_FW_PACKAGE_PUBLIC_KEY;
	status = manticore_measurements_measure_fw_pkg_key (fw_manifest, hash, &log->fw_pkg_public_key);
	if (status != 0) {
		return status;
	}

	/* Measure the SVN for the Firmware Package. */
	log->fw_pkg_svn.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_FW_PACKAGE_SVN;
	log->fw_pkg_svn.data.svn = manticore_firmware_descriptor_get_svn (fw_descriptor);

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fw_pkg_svn.data,
		sizeof (log->fw_pkg_svn.data), log->fw_pkg_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the build version number for the Firmware Package. */
	log->fw_pkg_version.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_FW_PACKAGE_BUILD_VERSION;
	memcpy (log->fw_pkg_version.data.build_version,
		manticore_firmware_descriptor_get_build_version (fw_descriptor),
		sizeof (log->fw_pkg_version.data.build_version));

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fw_pkg_version.data,
		sizeof (log->fw_pkg_version.data), log->fw_pkg_version.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the digest of the SPRT image data that was loaded from the Firmware Package. */
	log->sprt_image.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_SPRT_IMAGE;
	memcpy (log->sprt_image.data.digest.AsBytes, sp_digest->AsBytes, SP_MSG_384_SIZE);

	return hash->calculate_sha384 (hash, (uint8_t*) &log->sprt_image.data,
		sizeof (log->sprt_image.data), log->sprt_image.digest.AsBytes, SP_MSG_384_SIZE);
}

/**
 * Update an existing SP measurement log to include measurements for the AEB states.
 *
 * @param log The log that will be populated with AEB measurements.
 * @param hash Hash engine to use for calculating event digests.
 * @param aeb Driver interface to the device AEBs.
 *
 * @return 0 if the AEB measurements were generated successfully or an error code.
 */
int manticore_measurements_update_sp_log_with_aeb_state (struct manticore_measurements_log_sp *log,
	const struct hash_engine *hash, const struct hsp_aeb *aeb)
{
	int status;

	if ((log == NULL) || (hash == NULL) || (aeb == NULL)) {
		return MANTICORE_MEASUREMENTS_INVALID_ARGUMENT;
	}

	/* Measure the AEB state when exiting 1SP. */
	log->aeb_state.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_AEB_STATE;
	log->aeb_state.data.event.version = 0;
	status = aeb->get_multiple_aeb_state (aeb, log->aeb_state.data.aeb,
		ARRAY_SIZE (log->aeb_state.data.aeb));
	if (status != 0) {
		return status;
	}

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->aeb_state.data,
		sizeof (log->aeb_state.data), log->aeb_state.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the AEBs that were locked by 1SP. */
	log->aeb_locked.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_AEB_LOCKED;
	log->aeb_locked.data.event.version = 0;
	status = aeb->get_multiple_aeb_locked (aeb, log->aeb_locked.data.aeb,
		ARRAY_SIZE (log->aeb_locked.data.aeb));
	if (status != 0) {
		return status;
	}

	return hash->calculate_sha384 (hash, (uint8_t*) &log->aeb_locked.data,
		sizeof (log->aeb_locked.data), log->aeb_locked.digest.AsBytes, SP_MSG_384_SIZE);
}

/**
 * Generate a measurement log for the loaded CP, FP, and PCIe PHY firmware.
 *
 * @param log The log that will be populated with measurements.
 * @param hash Hash engine to use for calculating event digests.
 * @param fw_manifest The firmware key manifest used for firmware package verification.
 * @param fw_descriptor The firmware descriptor in the loaded firmware package.
 * @param cp_digest Digest of the complete CP firmware image.
 * @param fp0_digest Digest of the complete FP0 firmware image.
 * @param fp1_digest Digest of the complete FP1 firmware image.
 * @param fp2_digest Digest of the complete FP2 firmware image.
 * @param phy_digest Digest of the PCIe PHY firmware image.
 *
 * @return 0 if the SoC measurement log was generated successfully or an error code.
 */
int manticore_measurements_generate_soc_log (struct manticore_measurements_log_soc *log,
	const struct hash_engine *hash, const struct key_manifest_hsp_firmware *fw_manifest,
	const struct manticore_firmware_descriptor *fw_descriptor, const SP_MSG_384 *cp_digest,
	const SP_MSG_384 *fp0_digest, const SP_MSG_384 *fp1_digest, const SP_MSG_384 *fp2_digest,
	const SP_MSG_384 *phy_digest)
{
	int status;

	if ((log == NULL) || (hash == NULL) || (fw_manifest == NULL) || (fw_descriptor == NULL) ||
		(cp_digest == NULL) || (fp0_digest == NULL) || (fp1_digest == NULL) ||
		(fp2_digest == NULL) || (phy_digest == NULL)) {
		return MANTICORE_MEASUREMENTS_INVALID_ARGUMENT;
	}

	memset (log, 0, sizeof (struct manticore_measurements_log_soc));

	/* Measure the digest for the Firmware Key Manifest, which represents the list of keys used by
	 * device for various operations.  The assumption is that the digest in the header matches the
	 * contents of the payload.  If it didn't, the manifest would have already been rejected. */
	log->fw_key_manifest.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_CP_FW_KEY_MANIFEST;
	memcpy (log->fw_key_manifest.data.digest.AsBytes, fw_manifest->keys->header_signed.digest,
		SP_MSG_384_SIZE);

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fw_key_manifest.data,
		sizeof (log->fw_key_manifest.data), log->fw_key_manifest.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the SVN for the Firmware Key Manifest. */
	log->fw_key_manifest_svn.data.event.event_id =
		MANTICORE_MEASUREMENTS_EVENT_CP_FW_KEY_MANIFEST_SVN;
	log->fw_key_manifest_svn.data.svn = fw_manifest->keys->header_signed.svn;

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fw_key_manifest_svn.data,
		sizeof (log->fw_key_manifest_svn.data), log->fw_key_manifest_svn.digest.AsBytes,
		SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the public key from the Firmware Key Manifest that was used to verify the Firmware
	 * Package. */
	log->fw_pkg_public_key.data.event.event_id =
		MANTICORE_MEASUREMENTS_EVENT_CP_FW_PACKAGE_PUBLIC_KEY;
	status = manticore_measurements_measure_fw_pkg_key (fw_manifest, hash, &log->fw_pkg_public_key);
	if (status != 0) {
		return status;
	}

	/* Measure the SVN for the Firmware Package. */
	log->fw_pkg_svn.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_CP_FW_PACKAGE_SVN;
	log->fw_pkg_svn.data.svn = manticore_firmware_descriptor_get_svn (fw_descriptor);

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fw_pkg_svn.data,
		sizeof (log->fw_pkg_svn.data), log->fw_pkg_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the build version number for the Firmware Package. */
	log->fw_pkg_version.data.event.event_id =
		MANTICORE_MEASUREMENTS_EVENT_CP_FW_PACKAGE_BUILD_VERSION;
	memcpy (log->fw_pkg_version.data.build_version,
		manticore_firmware_descriptor_get_build_version (fw_descriptor),
		sizeof (log->fw_pkg_version.data.build_version));

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fw_pkg_version.data,
		sizeof (log->fw_pkg_version.data), log->fw_pkg_version.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the digest of the CP image data that was loaded from the Firmware Package. */
	log->cp_image.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_CP_IMAGE;
	memcpy (log->cp_image.data.digest.AsBytes, cp_digest->AsBytes, SP_MSG_384_SIZE);

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->cp_image.data,
		sizeof (log->cp_image.data), log->cp_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the digest of the FP core 0 image data that was loaded from the Firmware Package. */
	log->fp0_image.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_FP0_IMAGE;
	memcpy (log->fp0_image.data.digest.AsBytes, fp0_digest->AsBytes, SP_MSG_384_SIZE);

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fp0_image.data,
		sizeof (log->fp0_image.data), log->fp0_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the digest of the FP core 1 image data that was loaded from the Firmware Package. */
	log->fp1_image.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_FP1_IMAGE;
	memcpy (log->fp1_image.data.digest.AsBytes, fp1_digest->AsBytes, SP_MSG_384_SIZE);

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fp1_image.data,
		sizeof (log->fp1_image.data), log->fp1_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the digest of the FP core 2 image data that was loaded from the Firmware Package. */
	log->fp2_image.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_FP2_IMAGE;
	memcpy (log->fp2_image.data.digest.AsBytes, fp2_digest->AsBytes, SP_MSG_384_SIZE);

	status = hash->calculate_sha384 (hash, (uint8_t*) &log->fp2_image.data,
		sizeof (log->fp2_image.data), log->fp2_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Measure the digest of the PCIe PHY image data that was loaded from the Firmware Package. */
	log->phy_image.data.event.event_id = MANTICORE_MEASUREMENTS_EVENT_PCIE_PHY_IMAGE;
	memcpy (log->phy_image.data.digest.AsBytes, phy_digest->AsBytes, SP_MSG_384_SIZE);

	return hash->calculate_sha384 (hash, (uint8_t*) &log->phy_image.data,
		sizeof (log->phy_image.data), log->phy_image.digest.AsBytes, SP_MSG_384_SIZE);
}

/**
 * Extend a PCR using digests from a SP measurement log.  Measurements for AEB state will not be
 * extended to the PCR.
 *
 * @param log A log of measurements to extend the PCR with.  Each entry in the log will be extended
 * in order.
 * @param ccs Driver for the CCS that manages the PCR to extend.
 * @param pcr PCR number to extend with the SP measurements.
 *
 * @return 0 if all measurements were extended to the PCR successfully or an error code.
 */
int manticore_measurements_extend_sp_pcr (const struct manticore_measurements_log_sp *log,
	const struct ccs_ksu_interface *ccs, uint8_t pcr)
{
	int status;

	if ((log == NULL) || (ccs == NULL)) {
		return MANTICORE_MEASUREMENTS_INVALID_ARGUMENT;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->security_policy.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_key_manifest.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_key_manifest_svn.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_pkg_public_key.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_pkg_svn.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_pkg_version.digest);
	if (status != 0) {
		return status;
	}

	return ccs->extend_pcr (ccs, pcr, &log->sprt_image.digest);
}

/**
 * Extend a PCR using digests for AEB state from a SP measurement log.  Only AEB state measurements
 * will be extended.
 *
 * @param log A log of measurements to extend the PCR with.  Each entry in the log will be extended
 * in order.
 * @param ccs Driver for the CCS that manages the PCR to extend.
 * @param pcr PCR number to extend with the SP measurements.
 *
 * @return 0 if all measurements were extended to the PCR successfully or an error code.
 */
int manticore_measurements_extend_sp_pcr_with_aeb_state (
	const struct manticore_measurements_log_sp *log, const struct ccs_ksu_interface *ccs,
	uint8_t pcr)
{
	int status;

	if ((log == NULL) || (ccs == NULL)) {
		return MANTICORE_MEASUREMENTS_INVALID_ARGUMENT;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->aeb_state.digest);
	if (status != 0) {
		return status;
	}

	return ccs->extend_pcr (ccs, pcr, &log->aeb_locked.digest);
}

/**
 * Extend a single measurement from the log into the calculated PCR.
 *
 * @param hash Hash engine to use for PCR extension.
 * @param measurement The measurement digest to extend.
 * @param extend Buffer containing the current PCR state in the first 384 bits.  Upon return, it
 * will contain the new PCR state.
 *
 * @return 0 if the PCR was extended successfully or an error code.
 */
static int manticore_measurement_extend_pcr (const struct hash_engine *hash,
	const SP_MSG_384 *measurement, uint8_t extend[SHA384_HASH_LENGTH * 2])
{
	memcpy (&extend[SHA384_HASH_LENGTH], measurement->AsBytes, SHA384_HASH_LENGTH);

	return hash->calculate_sha384 (hash, extend, SHA384_HASH_LENGTH * 2, extend,
		SHA384_HASH_LENGTH);
}

/**
 * Verify the current state of the Manticore PCR matches the expected value based on the measurement
 * log for SP measurements from both ROM and 1SP.  This will only work correctly against a PCR that
 * has been extended with all the expected measurements.
 *
 * Neither the PCR nor the measurements logs will be modified.
 *
 * @param rom_log A log of measurements from ROM that were extended to the PCR.
 * @param sp_log A log of measurements from 1SP that were extended to the PCR.
 * @param hash Hash engine to use for PCR calculation.
 * @param ccs Driver for the CCS that manages the PCR to verify against.
 * @param pcr PCR number that is expected to match the log measurements.
 * @param is_reinit Flag to indicate if the PCR has been reinitialized after SoC reset, since this
 * action changes the initial PCR value.
 *
 * @return 0 if the PCR state matches the log or an error code.
 */
int manticore_measurements_verify_sp_pcr (const struct boot_measurements_single_root_log *rom_log,
	const struct manticore_measurements_log_sp *sp_log, const struct hash_engine *hash,
	const struct ccs_ksu_interface *ccs, uint8_t pcr, bool is_reinit)
{
	uint8_t extend[SHA384_HASH_LENGTH * 2];
	int status;

	if ((rom_log == NULL) || (sp_log == NULL) || (hash == NULL) || (ccs == NULL)) {
		return MANTICORE_MEASUREMENTS_INVALID_ARGUMENT;
	}

	/* Start with the initial PCR value set by HW. */
	memcpy (extend, MANTICORE_MEASUREMENTS_INIT_PCR_VALUE, SHA384_HASH_LENGTH);
	extend[0] = pcr;
	if (is_reinit) {
		extend[1] = 0x10;
	}

	/* Extend each measurement to get the PCR value. */

	/* ROM measurements */
	status = manticore_measurement_extend_pcr (hash, &rom_log->security_state.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &rom_log->owner_public_key.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &rom_log->key_manifest_svn.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &rom_log->tenancy_counter.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &rom_log->fw_public_key.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &rom_log->secondary_public_key.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &rom_log->fw_svn.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &rom_log->fw_version.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &rom_log->fw_image.digest, extend);
	if (status != 0) {
		return status;
	}

	/* 1SP measurements */
	status = manticore_measurement_extend_pcr (hash, &sp_log->security_policy.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &sp_log->fw_key_manifest.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &sp_log->fw_key_manifest_svn.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &sp_log->fw_pkg_public_key.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &sp_log->fw_pkg_svn.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &sp_log->fw_pkg_version.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &sp_log->sprt_image.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &sp_log->aeb_state.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &sp_log->aeb_locked.digest, extend);
	if (status != 0) {
		return status;
	}

	/* Compare the calculated PCR against the HW state. */
	status = ccs->get_pcr_value (ccs, pcr, (SP_MSG_384*) &extend[SHA384_HASH_LENGTH]);
	if (status != 0) {
		return status;
	}

	if (buffer_compare (extend, &extend[SHA384_HASH_LENGTH], SHA384_HASH_LENGTH) != 0) {
		return MANTICORE_MEASUREMENTS_PCR_MISMATCH;
	}

	return 0;
}

/**
 * Extend a PCR using digests from a SoC measurement log.
 *
 * @param log A log of measurements to extend the PCR with.  Each entry in the log will be extended
 * in order.
 * @param ccs Driver for the CCS that manages the PCR to extend.
 * @param pcr PCR number to extend with the SoC measurements.
 *
 * @return 0 if all measurements were extended to the PCR successfully or an error code.
 */
int manticore_measurements_extend_soc_pcr (const struct manticore_measurements_log_soc *log,
	const struct ccs_ksu_interface *ccs, uint8_t pcr)
{
	int status;

	if ((log == NULL) || (ccs == NULL)) {
		return MANTICORE_MEASUREMENTS_INVALID_ARGUMENT;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_key_manifest.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_key_manifest_svn.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_pkg_public_key.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_pkg_svn.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_pkg_version.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->cp_image.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fp0_image.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fp1_image.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fp2_image.digest);
	if (status != 0) {
		return status;
	}

	return ccs->extend_pcr (ccs, pcr, &log->phy_image.digest);
}

/**
 * Verify the current state of the Manticore PCR matches the expected value based on the measurement
 * log for SoC measurements.
 *
 * @param log A log of measurements that will be used to verify the PCR state.  The PCR will not be
 * modified.
 * @param hash Hash engine to use for PCR calculation.
 * @param ccs Driver for the CCS that manages the PCR to verify against.
 * @param pcr PCR number that is expected to match the log measurements.
 * @param is_reinit Flag to indicate if the PCR has been reinitialized after SoC reset, since this
 * action changes the initial PCR value.
 *
 * @return 0 if the PCR state matches the log or an error code.
 */
int manticore_measurements_verify_soc_pcr (const struct manticore_measurements_log_soc *log,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t pcr,
	bool is_reinit)
{
	uint8_t extend[SHA384_HASH_LENGTH * 2];
	int status;

	if ((log == NULL) || (hash == NULL) || (ccs == NULL)) {
		return MANTICORE_MEASUREMENTS_INVALID_ARGUMENT;
	}

	/* Start with the initial PCR value set by HW. */
	memcpy (extend, MANTICORE_MEASUREMENTS_INIT_PCR_VALUE, SHA384_HASH_LENGTH);
	extend[0] = pcr;
	if (is_reinit) {
		extend[1] = 0x10;
	}

	/* Extend each measurement to get the PCR value. */
	status = manticore_measurement_extend_pcr (hash, &log->fw_key_manifest.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &log->fw_key_manifest_svn.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &log->fw_pkg_public_key.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &log->fw_pkg_svn.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &log->fw_pkg_version.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &log->cp_image.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &log->fp0_image.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &log->fp1_image.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &log->fp2_image.digest, extend);
	if (status != 0) {
		return status;
	}

	status = manticore_measurement_extend_pcr (hash, &log->phy_image.digest, extend);
	if (status != 0) {
		return status;
	}

	/* Compare the calculated PCR against the HW state. */
	status = ccs->get_pcr_value (ccs, pcr, (SP_MSG_384*) &extend[SHA384_HASH_LENGTH]);
	if (status != 0) {
		return status;
	}

	if (buffer_compare (extend, &extend[SHA384_HASH_LENGTH], SHA384_HASH_LENGTH) != 0) {
		return MANTICORE_MEASUREMENTS_PCR_MISMATCH;
	}

	return 0;
}

/**
 * Generate a DICE FWID for the SPRT image based on the measurement log.  The FWID will be a hash
 * on the concatenation of all measurements, excluding the AEB state measurements.
 *
 * @param log The measurement log to use for FWID generation.
 * @param hash Hash engine to use for calculation.
 * @param fwid Output for the calculated FWID.
 *
 * @return 0 if the FWID was generated successfully or an error code.
 */
int manticore_measurements_generate_sprt_fwid (const struct manticore_measurements_log_sp *log,
	const struct hash_engine *hash, SP_MSG_384 *fwid)
{
	int status;

	if ((log == NULL) || (hash == NULL) || (fwid == NULL)) {
		return MANTICORE_MEASUREMENTS_INVALID_ARGUMENT;
	}

	status = hash->start_sha384 (hash);
	if (status != 0) {
		return status;
	}

	status = hash->update (hash, log->security_policy.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto exit;
	}

	status = hash->update (hash, log->fw_key_manifest.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto exit;
	}

	status = hash->update (hash, log->fw_key_manifest_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto exit;
	}

	status = hash->update (hash, log->fw_pkg_public_key.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto exit;
	}

	status = hash->update (hash, log->fw_pkg_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto exit;
	}

	status = hash->update (hash, log->fw_pkg_version.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto exit;
	}

	status = hash->update (hash, log->sprt_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto exit;
	}

	status = hash->finish (hash, fwid->AsBytes, SP_MSG_384_SIZE);

exit:
	if (status != 0) {
		hash->cancel (hash);
	}

	return status;
}
