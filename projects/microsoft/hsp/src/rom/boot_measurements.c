// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "boot_measurements.h"


/**
 * Generates the measurement log for security state information.
 *
 * @param state Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param fuses Interface to the device fuses.
 * @param a0_bypass Current state of the A0 Bypass signal to the device.
 * @param reset_type Bitmask indicating what caused the device reset.  A value of 0 will indicate a
 * POR, while any other value indicates a warm reset.  This value will typically just be the raw
 * value read from HSP_FATAL_ERR_LOG in the CREG block.
 * @param rng_cal The actual calibration that was applied to the RNG.  This could differ from the
 * raw values stored in fuses if the fused values are outside any limits being applied by the RNG
 * driver.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_security_state (
	struct boot_measurements_security_state *state, const struct hash_engine *hash,
	const struct fuse_controller_interface *fuses, uint8_t a0_bypass, uint32_t reset_type,
	const uint8_t rng_cal[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	if ((state == NULL) || (hash == NULL) || (fuses == NULL) ||
		(rng_cal == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	/* Measure the base security configuration of the device.  This includes the life-cycle security
	 * state, the calibration applied to the HW RNG, and the state of the A0 Bypass pin. */
	state->data.event.event_id = BOOT_MEASUREMENTS_EVENT_SECURITY_STATE;
	state->data.security_state = fuses->get_security_state (fuses);
	state->data.a0_bypass = a0_bypass;
	state->data.reset_type = reset_type;
	memcpy (state->data.rng_calibration, rng_cal, FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);

	return hash->calculate_sha384 (hash, (uint8_t*) &state->data, sizeof (state->data),
		state->digest.AsBytes, SP_MSG_384_SIZE);
}

/**
 * Generates the measurement log for a public key.
 *
 * @param key_out Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param key The incoming key that will be used for measurements.
 * @param event Which public key event this is. If not a public key event, will fail.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_public_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct key_manifest_public_key *key, uint32_t event)
{
	if ((key_out == NULL) || (hash == NULL)) {
		/* Key may be null depending on the implementation in the log,
		 * so do not fail that here. */
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	key_out->data.event.event_id = event;
	if (key != NULL) {
		memcpy (key_out->data.key.Parts.X.AsBytes, key->key.ecc->x, SP_MSG_384_SIZE);
		memcpy (key_out->data.key.Parts.Y.AsBytes, key->key.ecc->y, SP_MSG_384_SIZE);
	}

	return hash->calculate_sha384 (hash, (uint8_t*) &key_out->data, sizeof (key_out->data),
		key_out->digest.AsBytes, SP_MSG_384_SIZE);
}

/**
 * Generates the measurement log for the primary root key.
 *
 * @param key_out Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param rot Interface to the current RoT state.
 * @param keys The key manifest for the firmware image that was validated and loaded.
 * @param hw_key_out Output root key status.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_primary_root_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct hw_rot *rot, const struct key_manifest_hsp_rom *keys, int *hw_key_out)
{
	const struct key_manifest_public_key *key = NULL;

	if ((key_out == NULL) || (hash == NULL) || (rot == NULL) ||
		(keys == NULL) || (hw_key_out == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	*hw_key_out = rot->has_root_key (rot);
	if (*hw_key_out == 0) {
		key = keys->base.get_root_key (&keys->base);
		if (key == NULL) {
			return BOOT_MEASUREMENTS_NO_ROOT_KEY;
		}
	}
	else if ((*hw_key_out != HW_ROT_UNSUPPORTED) && (*hw_key_out != HW_ROT_NO_ROOT_KEY)) {
		return *hw_key_out;
	}

	return boot_measurements_generate_log_public_key (key_out, hash, key,
		BOOT_MEASUREMENTS_EVENT_OWNER_PUBLIC_KEY);
}

/**
 * Generates the measurement log for the primary firmware key.
 *
 * @param key_out Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param keys The key manifest for the firmware image that was validated and loaded.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_primary_firmware_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct key_manifest_hsp_rom *keys)
{
	const struct key_manifest_public_key *key = NULL;

	if ((key_out == NULL) || (hash == NULL) || (keys == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	key = keys->base.get_app_key (&keys->base);
	if (key == NULL) {
		return BOOT_MEASUREMENTS_NO_FW_KEY;
	}

	return boot_measurements_generate_log_public_key (key_out, hash, key,
		BOOT_MEASUREMENTS_EVENT_FW_PUBLIC_KEY);
}

/**
 * Generates the measurement log for the secondary firmware key.
 *
 * @param key_out Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param keys The key manifest for the firmware image that was validated and loaded.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_secondary_firmware_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct key_manifest_hsp_rom *keys)
{
	const struct key_manifest_public_key *key = NULL;

	if ((key_out == NULL) || (hash == NULL) || (keys == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	key = keys->get_secondary_key (keys);

	return boot_measurements_generate_log_public_key (key_out, hash, key,
		BOOT_MEASUREMENTS_EVENT_SECONDARY_PUBLIC_KEY);
}

/**
 * Generates the measurement log for the tenancy grant key.
 *
 * @param key_out Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param keys The key manifest for the firmware image that was validated and loaded.
 * @param required Flag for if the tenancy grant key is required.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_tenancy_grant_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct key_manifest_hsp_rom *keys, bool required)
{
	const struct key_manifest_public_key *key = NULL;

	if ((key_out == NULL) || (hash == NULL) || (keys == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	key = keys->get_tenancy_grant_key (keys);
	if (required && (key == NULL)) {
		return BOOT_MEASUREMENTS_NO_TENANCY_KEY;
	}

	return boot_measurements_generate_log_public_key (key_out, hash, key,
		BOOT_MEASUREMENTS_EVENT_TENANCY_GRANT_KEY);
}

/**
 * Generates the measurement log for an SVN.
 *
 * @param svn Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param keys The key manifest for the firmware image that was validated and loaded.
 * @param hw_key The HW key status based on previous steps.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_manifest_svn (struct boot_measurements_svn *svn,
	const struct hash_engine *hash, const struct key_manifest_hsp_rom *keys, int hw_key)
{
	if ((svn == NULL) || (hash == NULL) || (keys == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	/* Measure the SVN reported by the key manifest use to verify the firmware image.  If there is
	 * no hardware enforcement of the SVN, a value of 0 will be measured instead. */
	svn->data.event.event_id = BOOT_MEASUREMENTS_EVENT_KEY_MANIFEST_SVN;
	if (hw_key == 0) {
		svn->data.svn = keys->get_svn (keys);
	}

	return hash->calculate_sha384 (hash, (uint8_t*) svn, sizeof (svn->data), svn->digest.AsBytes,
		SP_MSG_384_SIZE);
}

/**
 * Generates the measurement log for a tenancy counter.
 *
 * @param tenancy Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param rot Interface to the current RoT state.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_manifest_tenancy_counter (
	struct boot_measurements_tenancy_counter *tenancy, const struct hash_engine *hash,
	const struct hw_rot *rot)
{
	int status;

	if ((tenancy == NULL) || (hash == NULL) || (rot == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	/* Measure value of the tenancy counter.  This ensures the tenancy state impacts the device
	 * identity key. */
	tenancy->data.event.event_id = BOOT_MEASUREMENTS_EVENT_TENANCY_COUNTER;
	status = rot->get_tenancy_counter (rot, tenancy->data.counter, sizeof (tenancy->data.counter));
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return hash->calculate_sha384 (hash, (uint8_t*) tenancy, sizeof (tenancy->data),
		tenancy->digest.AsBytes, SP_MSG_384_SIZE);
}

/**
 * Generates the measurement log for the SVN of the firmware image being loaded.
 *
 * @param svn Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param fw The firmware image that was validated and loaded into memory.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_firmware_svn (struct boot_measurements_svn *svn,
	const struct hash_engine *hash, const struct hsp_fw_1sp *fw)
{
	if ((svn == NULL) || (hash == NULL) || (fw == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	/* Measure the SVN reported by the loaded firmware image. */
	svn->data.event.event_id = BOOT_MEASUREMENTS_EVENT_FW_SVN;
	svn->data.svn = fw->state->header.header_signed.svn;

	return hash->calculate_sha384 (hash, (uint8_t*) svn, sizeof (svn->data), svn->digest.AsBytes,
		SP_MSG_384_SIZE);
}

/**
 * Generates the measurement log for the version of the firmware image being loaded.
 *
 * @param fw_version Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param fw The firmware image that was validated and loaded into memory.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_firmware_version (
	struct boot_measurements_build_version *fw_version, const struct hash_engine *hash,
	const struct hsp_fw_1sp *fw)
{
	if ((fw_version == NULL) || (hash == NULL) || (fw == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	/* Measure the build version reported by the loaded firmware image. */
	fw_version->data.event.event_id = BOOT_MEASUREMENTS_EVENT_BUILD_VERSION;
	memcpy (fw_version->data.build_version, fw->state->header.header_signed.build_version,
		HSP_FW_1SP_VERSION_LEN);

	return hash->calculate_sha384 (hash, (uint8_t*) fw_version, sizeof (fw_version->data),
		fw_version->digest.AsBytes, SP_MSG_384_SIZE);
}

/**
 * Generates the measurement log for the firmware image being loaded.
 *
 * @param fw_image Log entry that will be populated with the measurement.
 * @param hash Hash engine to use to generate the measurements.
 * @param fw The firmware image that was validated and loaded into memory.
 *
 * @return 0 if this portion of the log was generated successfully or an error code.
 */
int boot_measurements_generate_log_firmware_image (struct boot_measurements_digest *fw_image,
	const struct hash_engine *hash, const struct hsp_fw_1sp *fw)
{
	if ((fw_image == NULL) || (hash == NULL) || (fw == NULL)) {
		return BOOT_MEASUREMENTS_INVALID_ARGUMENT;
	}

	/* Measure the digest of the firmware image binary loaded into memory.  We make the assumption
	 * that the digest in the header matches the image that was loaded.  If it didn't, we wouldn't
	 * be at this point in the boot process. */
	fw_image->data.event.event_id = BOOT_MEASUREMENTS_EVENT_FW_IMAGE;
	memcpy (fw_image->data.digest.AsBytes, fw->state->header.header_signed.digest.AsBytes,
		SP_MSG_384_SIZE);

	return hash->calculate_sha384 (hash, (uint8_t*) fw_image, sizeof (fw_image->data),
		fw_image->digest.AsBytes, SP_MSG_384_SIZE);
}
