// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "boot_measurements.h"
#include "boot_measurements_single_root.h"


/**
 * Generate the measurement log for the loaded firmware image.
 *
 * @param log Log that will be populated with measured entries.
 * @param hash Hash engine to use to generate the measurements.
 * @param fuses Interface to the device fuses.
 * @param a0_bypass Current state of the A0 Bypass signal to the device.
 * @param reset_type Bitmask indicating what caused the device reset.  A value of 0 will indicate a
 * POR, while any other value indicates a warm reset.  This value will typically just be the raw
 * value read from HSP_FATAL_ERR_LOG in the CREG block.
 * @param rng_cal The actual calibration that was applied to the RNG.  This could differ from the
 * raw values stored in fuses if the fused values are outside any limits being applied by the RNG
 * driver.
 * @param rot Interface to the current RoT state.
 * @param keys The key manifest for the firmware image that was validated and loaded.
 * @param fw The firmware image that was validated and loaded into memory.
 *
 * @return 0 if the log was generated successfully or an error code.
 */
int boot_measurements_single_root_generate_log (struct boot_measurements_single_root_log *log,
	const struct hash_engine *hash, const struct fuse_controller_interface *fuses,
	uint8_t a0_bypass, uint32_t reset_type,
	const uint8_t rng_cal[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH], const struct hw_rot *rot,
	const struct key_manifest_hsp_rom *keys, const struct hsp_fw_1sp *fw)
{
	int hw_key;
	int status;

	if ((log == NULL) || (hash == NULL) || (fuses == NULL) || (rng_cal == NULL) || (rot == NULL) ||
		(keys == NULL) || (fw == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	memset (log, 0, sizeof (struct boot_measurements_single_root_log));

	status = boot_measurements_generate_log_security_state (&log->security_state, hash, fuses,
		a0_bypass, reset_type, rng_cal);
	if (status != 0) {
		return status;
	}

	status = boot_measurements_generate_log_primary_root_key (&log->owner_public_key, hash, rot,
		keys, &hw_key);
	if (status != 0) {
		return status;
	}

	status = boot_measurements_generate_log_manifest_svn (&log->key_manifest_svn, hash, keys,
		hw_key);
	if (status != 0) {
		return status;
	}

	status = boot_measurements_generate_log_manifest_tenancy_counter (&log->tenancy_counter, hash,
		rot);
	if (status != 0) {
		return status;
	}

	status = boot_measurements_generate_log_primary_firmware_key (&log->fw_public_key, hash, keys);
	if (status != 0) {
		return status;
	}

	/* Measure any additional public key used during firmware image verification. */
	if (keys->is_tenancy_grant (keys)) {
		status = boot_measurements_generate_log_tenancy_grant_key (&log->secondary_public_key, hash,
			keys, true);
		if (status != 0) {
			return status;
		}
	}
	else {
		status = boot_measurements_generate_log_secondary_firmware_key (&log->secondary_public_key,
			hash, keys);
		if (status != 0) {
			return status;
		}
	}

	status = boot_measurements_generate_log_firmware_svn (&log->fw_svn, hash, fw);
	if (status != 0) {
		return status;
	}

	status = boot_measurements_generate_log_firmware_version (&log->fw_version, hash, fw);
	if (status != 0) {
		return status;
	}

	return boot_measurements_generate_log_firmware_image (&log->fw_image, hash, fw);
}

/**
 * Extend a PCR using digests from a measurement log.
 *
 * @param log A log of measurements to extend the PCR with.  Each entry in the log will be extended
 * in order.
 * @param ccs Driver for the CCS that manages the PCR to extend.
 * @param pcr PCR number to extend with the boot measurements.
 *
 * @return 0 if all measurements were extended to the PCR successfully or an error code.
 */
int boot_measurements_single_root_extend_pcr (const struct boot_measurements_single_root_log *log,
	const struct ccs_ksu_interface *ccs, uint8_t pcr)
{
	int status;

	if ((log == NULL) || (ccs == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->security_state.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->owner_public_key.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->key_manifest_svn.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->tenancy_counter.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_public_key.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->secondary_public_key.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_svn.digest);
	if (status != 0) {
		return status;
	}

	status = ccs->extend_pcr (ccs, pcr, &log->fw_version.digest);
	if (status != 0) {
		return status;
	}

	return ccs->extend_pcr (ccs, pcr, &log->fw_image.digest);
}

/**
 * Calculate a FWID digest for the boot image that only contains the measurements that do not vary
 * between devices.  Specifically, this means the digest will exclude the Tenancy Counter
 * measurement.
 *
 * @param log The log of boot measurements that will be hashed to generate the FWID digest.
 * @param hash The hash engine to use for FWID digest calculation.
 * @param fwid Output for the FWID digest.
 *
 * @return 0 if the FWID digest was calculated successfully or an error code.
 */
int boot_measurements_single_root_generate_device_independent_fwid (
	const struct boot_measurements_single_root_log *log, const struct hash_engine *hash,
	SP_MSG_384 *fwid)
{
	int status;

	if ((log == NULL) || (hash == NULL) || (fwid == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	status = hash->start_sha384 (hash);
	if (status != 0) {
		return status;
	}

	status = hash->update (hash, log->security_state.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto cancel;
	}

	status = hash->update (hash, log->owner_public_key.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto cancel;
	}

	status = hash->update (hash, log->key_manifest_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto cancel;
	}

	status = hash->update (hash, log->fw_public_key.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto cancel;
	}

	status = hash->update (hash, log->secondary_public_key.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto cancel;
	}

	status = hash->update (hash, log->fw_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto cancel;
	}

	status = hash->update (hash, log->fw_version.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto cancel;
	}

	status = hash->update (hash, log->fw_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto cancel;
	}

	status = hash->finish (hash, fwid->AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto cancel;
	}

	return 0;

cancel:
	hash->cancel (hash);

	return status;
}
