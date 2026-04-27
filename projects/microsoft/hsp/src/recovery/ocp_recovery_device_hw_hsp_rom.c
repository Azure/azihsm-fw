// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "ocp_recovery_device_hw_hsp_rom.h"
#include "common/unused.h"
#include "firmware/hsp_fw_1sp.h"
#include "firmware/key_manifest_hsp_rom.h"
#include "logging/code_path_integrity.h"
#include "rom/load_image_1sp.h"
#include "rom/rom_logging.h"
#include "splibs/inc/spchkptdefs.h"


/* The checkpoint chain required for recovery image verification. */
extern const HSP_CHKPT_CONFIG ROM_CHECKPOINT_START_RECOVERY_LOAD;
extern const HSP_CHKPT_CONFIG ROM_CHECKPOINT_RECOVERY_VERIFY;
extern const HSP_CHKPT_CONFIG ROM_CHECKPOINT_HAND_OFF_RECOVERY_NEXT;


int ocp_recovery_device_hw_hsp_rom_get_device_id (const struct ocp_recovery_device_hw *recovery_hw,
	struct ocp_recovery_device_id *id)
{
	const struct ocp_recovery_device_hw_hsp_rom *rom =
		(const struct ocp_recovery_device_hw_hsp_rom*) recovery_hw;
	uint32_t socid[4];

	if ((rom == NULL) || (id == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	/* Make a temp copy since SOCID accesses need to be word-aligned. */
	socid[0] = rom->socid[0];
	socid[1] = rom->socid[1];
	socid[2] = rom->socid[2];
	socid[3] = rom->socid[3];

	id->base.id_type = OCP_RECOVERY_DEVICE_ID_UUID;
	id->base.vendor_length = 0;
	memcpy (id->base.uuid.uuid, socid, sizeof (id->base.uuid.uuid));
	memset (id->base.uuid.pad, 0, sizeof (id->base.uuid.pad));

	return sizeof (id->base);
}

void ocp_recovery_device_hw_hsp_rom_get_device_status (
	const struct ocp_recovery_device_hw *recovery_hw,
	enum ocp_recovery_device_status_code *status_code,
	enum ocp_recovery_recovery_reason_code *reason_code,
	struct ocp_recovery_device_status_vendor *vendor)
{
	const struct ocp_recovery_device_hw_hsp_rom *rom =
		(const struct ocp_recovery_device_hw_hsp_rom*) recovery_hw;

	if ((rom == NULL) || (status_code == NULL) || (reason_code == NULL) || (vendor == NULL)) {
		return;
	}

	rom_logging_get_last_error (vendor);

	/* Determine overall status based on the supported features of the interface and the last error
	 * information. */
	if (rom->base.activate_recovery) {
		/* If recovery images are supported, the device is in recovery mode. */
		if (vendor->error_code == 0) {
			/* There is no error, so this must be a forced recovery scenario. */
			*status_code = OCP_RECOVERY_DEVICE_STATUS_RECOVERY_MODE;
			*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_FORCED_RECOVERY;
		}
		else {
			/* The only other way into recovery mode is due to boot errors. */
			*status_code = OCP_RECOVERY_DEVICE_STATUS_BOOT_FAILURE;

			/* Determine a detailed reason code, if possible. */
			switch (vendor->error_code) {
				case KEY_MANIFEST_INVALID_FORMAT:
					*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_NO_KEY_MANIFEST;
					break;

				case KEY_MANIFEST_VERIFY_FAILED:
				case KEY_MANIFEST_BAD_ROOT_KEY:
					*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_MAIFEST_AUTH_FAIL;
					break;

				case KEY_MANIFEST_REVOKED:
				case KEY_MANIFEST_ID_TOO_HIGH:
				case LOAD_IMAGE_1SP_MANIFEST_REVOKED:
					*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_MANIFEST_REVOKED;
					break;

				case HSP_FW_1SP_BAD_IMAGE_MARKER:
				case HSP_FW_1SP_IMAGE_NO_DATA:
					*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_NO_BOOT_LOADER;
					break;

				case HSP_FW_1SP_IMAGE_VERIFY_FAILED:
				case HSP_FW_1SP_IMAGE_NOT_BLOCK_ALIGNED:
				case FIRMWARE_LOADER_INVALID_ADDR:
				case FIRMWARE_LOADER_IMAGE_TOO_LARGE:
				case ECC_HW_ECDSA_BAD_SIGNATURE:	// Can't distinguish between manifest and 1SP
					*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_BOOT_AUTH_FAIL;
					break;

				case HSP_FW_1SP_SVN_MISMATCH:
					*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_BOOT_REVOKED;
					break;

				default:
					/* Some other error happened.  Report a generic error. */
					*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_SOFT_HW_ERROR;
					break;
			}
		}
	}
	else {
		/* When recovery is not supported, there must have been a fatal initialization error. */
		*status_code = OCP_RECOVERY_DEVICE_STATUS_FATAL_ERROR;

		/* If the error was due to KAT failures, report that in the reason code. */
		if (vendor->failure_id == ROM_LOGGING_FAIL_KAT) {
			*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_BIST_FAILURE;
		}
		else {
			*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_HW_ERROR;
		}
	}
}

void ocp_recovery_device_hw_hsp_rom_reset_management (
	const struct ocp_recovery_device_hw *recovery_hw, bool forced_recovery)
{
	const struct ocp_recovery_device_hw_hsp_rom *rom =
		(const struct ocp_recovery_device_hw_hsp_rom*) recovery_hw;

	if (rom == NULL) {
		return;
	}

	rom->reset ((rom->base.supports_forced_recovery) ? forced_recovery : false);
}

int ocp_recovery_device_hw_hsp_rom_activate_recovery (
	const struct ocp_recovery_device_hw *recovery_hw,
	const struct ocp_recovery_device_cms *recovery, bool *is_auth_error)
{
	const struct ocp_recovery_device_hw_hsp_rom *rom =
		(const struct ocp_recovery_device_hw_hsp_rom*) recovery_hw;
	int status;

	if (is_auth_error) {
		*is_auth_error = false;
	}

	if ((rom == NULL) || (recovery == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	code_path_integrity_message_trace (ROM_LOGGING_TRACE_LOAD_IMAGE_REC);

	code_path_integrity_checkpoint (&ROM_CHECKPOINT_RECOVERY_VERIFY);

	status = load_image_1sp_from_memory (recovery->base_addr, rom->pka, rom->hash, rom->manifest,
		rom->image);
	if (status == 0) {
		rom->state->image_loaded = true;
	}
	else {
		rom->state->image_loaded = false;
		rom_logging_error (ROM_LOGGING_FAIL_RECOVERY, status);

		/* The image failed verification, so go back and wait for a different one. */
		code_path_integrity_checkpoint_hand_off (&ROM_CHECKPOINT_HAND_OFF_RECOVERY_NEXT,
			&ROM_CHECKPOINT_START_RECOVERY_LOAD);

		if (is_auth_error) {
			switch (status) {
				case KEY_MANIFEST_INVALID_FORMAT:
				case KEY_MANIFEST_VERIFY_FAILED:
				case KEY_MANIFEST_BAD_ROOT_KEY:
				case KEY_MANIFEST_REVOKED:
				case KEY_MANIFEST_ID_TOO_HIGH:
				case LOAD_IMAGE_1SP_MANIFEST_REVOKED:
				case HSP_FW_1SP_BAD_IMAGE_MARKER:
				case HSP_FW_1SP_IMAGE_NOT_BLOCK_ALIGNED:
				case HSP_FW_1SP_IMAGE_NO_DATA:
				case HSP_FW_1SP_IMAGE_VERIFY_FAILED:
				case HSP_FW_1SP_SVN_MISMATCH:
				case FIRMWARE_LOADER_INVALID_ADDR:
				case FIRMWARE_LOADER_IMAGE_TOO_LARGE:
				case ECC_HW_ECDSA_BAD_SIGNATURE:
					*is_auth_error = true;
					break;
			}
		}
	}

	return status;
}

/**
 * Initialize an HSP ROM handler for OCP recovery actions.
 *
 * @param recovery The recovery handler to initialize.
 * @param state Variable context for the handler.  This must not already be initialized.
 * @param manifest Key manifest handler for parsing manifests on recovery images.
 * @param image 1SP firmware image handler for loading and verifying a recovery image.
 * @param pka The PKA engine to use for signature verification.
 * @param hash The hash engine to use for image verification.
 * @param socid The SOCID for the device.  This must be 16 bytes long.
 * @param reset Callback to use to trigger a device reset from the recovery interface.
 * @param force_recovery Flag indicating if forced recovery is supported for the device.
 *
 * @return 0 if the recovery handler was initialized successfully or an error code.
 */
int ocp_recovery_device_hw_hsp_rom_init (struct ocp_recovery_device_hw_hsp_rom *recovery,
	struct ocp_recovery_device_hw_hsp_rom_state *state, const struct key_manifest_hsp_rom *manifest,
	const struct hsp_fw_1sp *image, const struct ecc_hw *pka, const struct hash_engine *hash,
	const uint32_t *socid, void (*reset) (bool), bool force_recovery)
{
	if ((recovery == NULL) || (state == NULL) || (manifest == NULL) || (image == NULL) ||
		(pka == NULL) || (hash == NULL) || (socid == NULL) || (reset == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	memset (recovery, 0, sizeof (struct ocp_recovery_device_hw_hsp_rom));

	recovery->base.get_device_id = ocp_recovery_device_hw_hsp_rom_get_device_id;
	recovery->base.get_device_status = ocp_recovery_device_hw_hsp_rom_get_device_status;
	recovery->base.reset_management = ocp_recovery_device_hw_hsp_rom_reset_management;
	recovery->base.activate_recovery = ocp_recovery_device_hw_hsp_rom_activate_recovery;
	recovery->base.supports_forced_recovery = force_recovery;

	recovery->state = state;
	recovery->manifest = manifest;
	recovery->image = image;
	recovery->pka = pka;
	recovery->hash = hash;
	recovery->socid = socid;
	recovery->reset = reset;

	return ocp_recovery_device_hw_hsp_rom_init_state (recovery);
}

/**
 * Initialize an HSP ROM handler for OCP recovery actions.  The handler will only report status and
 * cannot be used to activate recovery images.
 *
 * @param recovery The recovery handler to initialize.
 * @param state Variable context for the handler.  This must not already be initialized.
 * @param socid The SOCID for the device.  This must be 16 bytes long.
 * @param reset Callback to use to trigger a device reset from the recovery interface.
 * @param force_recovery Flag indicating if forced recovery is supported for the device.
 *
 * @return 0 if the recovery handler was initialized successfully or an error code.
 */
int ocp_recovery_device_hw_hsp_rom_init_no_recovery (
	struct ocp_recovery_device_hw_hsp_rom *recovery,
	struct ocp_recovery_device_hw_hsp_rom_state *state, const uint32_t *socid, void (*reset) (bool),
	bool force_recovery)
{
	if ((recovery == NULL) || (state == NULL) || (socid == NULL) || (reset == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	memset (recovery, 0, sizeof (struct ocp_recovery_device_hw_hsp_rom));

	recovery->base.get_device_id = ocp_recovery_device_hw_hsp_rom_get_device_id;
	recovery->base.get_device_status = ocp_recovery_device_hw_hsp_rom_get_device_status;
	recovery->base.reset_management = ocp_recovery_device_hw_hsp_rom_reset_management;
	recovery->base.activate_recovery = NULL;
	recovery->base.supports_forced_recovery = force_recovery;

	recovery->state = state;
	recovery->manifest = NULL;
	recovery->image = NULL;
	recovery->pka = NULL;
	recovery->hash = NULL;
	recovery->socid = socid;
	recovery->reset = reset;

	return ocp_recovery_device_hw_hsp_rom_init_state (recovery);
}

/**
 * Initialize only the variable state of an HSP ROM OCP recovery handler.  The rest of the handler
 * structure is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param recovery The recovery handler that contains the state to initialize.
 *
 * @return 0 if the handler state was successfully initialized or an error code.
 */
int ocp_recovery_device_hw_hsp_rom_init_state (
	const struct ocp_recovery_device_hw_hsp_rom *recovery)
{
	if ((recovery == NULL) || (recovery->state == NULL) || (recovery->socid == NULL) ||
		(recovery->reset == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	/* The pieces for firmware validation are only necessary if it is possible to activate a
	 * recovery image. */
	if (recovery->base.activate_recovery &&
		((recovery->manifest == NULL) || (recovery->image == NULL) || (recovery->pka == NULL) ||
		(recovery->hash == NULL))) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	memset (recovery->state, 0, sizeof (struct ocp_recovery_device_hw_hsp_rom_state));

	return 0;
}

/**
 * Release the resources used for an HSP ROM OCP recovery handler.
 *
 * @param recovery The recovery handler to release.
 */
void ocp_recovery_device_hw_hsp_rom_release (const struct ocp_recovery_device_hw_hsp_rom *recovery)
{
	UNUSED (recovery);
}

/**
 * Reset the flag that indicates a recovery image was loaded into memory and validated.
 *
 * @param recovery The recovery handler to update.
 */
void ocp_recovery_device_hw_hsp_rom_clear_image_loaded (
	const struct ocp_recovery_device_hw_hsp_rom *recovery)
{
	if (recovery) {
		recovery->state->image_loaded = false;
	}
}

/**
 * Determine if a recovery image has been loaded into memory and validated.
 *
 * @param recovery The recovery handler to query.
 *
 * @return true if there is a validated image in memory ready to execute or false if not.
 */
bool ocp_recovery_device_hw_hsp_rom_is_image_loaded (
	const struct ocp_recovery_device_hw_hsp_rom *recovery)
{
	if (recovery) {
		return recovery->state->image_loaded;
	}
	else {
		return false;
	}
}
