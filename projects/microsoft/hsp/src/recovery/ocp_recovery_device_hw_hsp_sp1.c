// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "ocp_recovery_device_hw_hsp_rom_static.h"
#include "ocp_recovery_device_hw_hsp_sp1.h"
#include "common/unused.h"

void ocp_recovery_device_hw_hsp_sp1_get_device_status (
	const struct ocp_recovery_device_hw *recovery_hw,
	enum ocp_recovery_device_status_code *status_code,
	enum ocp_recovery_recovery_reason_code *reason_code,
	struct ocp_recovery_device_status_vendor *vendor)
{
	const struct ocp_recovery_device_hw_hsp_sp1 *sp1 =
		(const struct ocp_recovery_device_hw_hsp_sp1*) recovery_hw;

	if ((sp1 == NULL) || (status_code == NULL) || (reason_code == NULL) || (vendor == NULL)) {
		return;
	}

	vendor->failure_id = sp1->state->last_fail_id;
	vendor->error_code = sp1->state->last_error_code;

	/* Determine overall status based on the supported features of the interface and the last error
	 * information. */
	if (sp1->base.base.activate_recovery) {
		/* If recovery images are supported, the device is in recovery mode. */
		if (vendor->error_code == 0) {
			/* There is no error, so this must be a forced recovery scenario. */
			*status_code = OCP_RECOVERY_DEVICE_STATUS_RUNNING_RECOVERY;
			*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_FORCED_RECOVERY;
		}
		else {
			/* The only other way into recovery mode is due to boot errors. */
			*status_code = OCP_RECOVERY_DEVICE_STATUS_BOOT_FAILURE;

			/* Determine a detailed reason code, if possible. */
			switch (vendor->error_code) {
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
		*reason_code = OCP_RECOVERY_DEVICE_STATUS_REC_HW_ERROR;
	}
}

int ocp_recovery_device_hw_hsp_sp1_activate_recovery (
	const struct ocp_recovery_device_hw *recovery_hw,
	const struct ocp_recovery_device_cms *recovery, bool *is_auth_error)
{
	const struct ocp_recovery_device_hw_hsp_sp1 *sp1 =
		(const struct ocp_recovery_device_hw_hsp_sp1*) recovery_hw;

	UNUSED (is_auth_error);
	UNUSED (recovery);

	if (sp1 == NULL) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	sp1->base.state->image_loaded = true;

	return 0;
}

/**
 * Initialize an HSP SP1 handler for OCP recovery actions.
 *
 * @param recovery The recovery handler to initialize.
 * @param rom_state ROM variable context for the handler.  This must not already be initialized.
 * @param manifest Key manifest handler for parsing manifests on recovery images.
 * @param image 1SP firmware image handler for loading and verifying a recovery image.
 * @param pka The PKA engine to use for signature verification.
 * @param hash The hash engine to use for image verification.
 * @param socid The SOCID for the device.  This must be 16 bytes long.
 * @param reset Callback to use to trigger a device reset from the recovery interface.
 * @param rom_state SP1 variable context for the handler.  This must not already be initialized.
 *
 * @return 0 if the recovery handler was initialized successfully or an error code.
 */
int ocp_recovery_device_hw_hsp_sp1_init (struct ocp_recovery_device_hw_hsp_sp1 *recovery,
	struct ocp_recovery_device_hw_hsp_rom_state *rom_state,
	const struct key_manifest_hsp_rom *manifest, const struct ecc_hw *pka,
	const struct hash_engine *hash,	const uint32_t *socid, void (*reset) (bool),
	struct ocp_recovery_device_hw_hsp_sp1_state *sp1_state)
{
	if ((recovery == NULL) || (rom_state == NULL) || (manifest == NULL) ||
		(pka == NULL) || (hash == NULL) || (socid == NULL) || (reset == NULL) ||
		(sp1_state == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	memset (recovery, 0, sizeof (struct ocp_recovery_device_hw_hsp_sp1));

	recovery->base.base.get_device_id = ocp_recovery_device_hw_hsp_rom_get_device_id;
	recovery->base.base.get_device_status = ocp_recovery_device_hw_hsp_sp1_get_device_status;
	recovery->base.base.reset_management = ocp_recovery_device_hw_hsp_rom_reset_management;
	recovery->base.base.activate_recovery = ocp_recovery_device_hw_hsp_sp1_activate_recovery;
	recovery->base.base.supports_forced_recovery = true;

	recovery->base.state = rom_state;
	recovery->base.manifest = manifest;
	recovery->base.image = NULL;
	recovery->base.pka = pka;
	recovery->base.hash = hash;
	recovery->base.socid = socid;
	recovery->base.reset = reset;
	recovery->state = sp1_state;

	return ocp_recovery_device_hw_hsp_sp1_init_state (recovery);
}

/**
 * Initialize only the variable state of an HSP SP1 OCP recovery handler.  The rest of the handler
 * structure is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param recovery The recovery handler that contains the state to initialize.
 *
 * @return 0 if the handler state was successfully initialized or an error code.
 */
int ocp_recovery_device_hw_hsp_sp1_init_state (
	const struct ocp_recovery_device_hw_hsp_sp1 *recovery)
{
	if ((recovery == NULL) || (recovery->state == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	if ((recovery->base.state == NULL) || (recovery->base.socid == NULL) ||
		(recovery->base.reset == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	/* The pieces for firmware validation are only necessary if it is possible to activate a
	 * recovery image. */
	if (recovery->base.base.activate_recovery &&
		((recovery->base.manifest == NULL) || (recovery->base.pka == NULL) ||
		(recovery->base.hash == NULL))) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	memset (recovery->state, 0, sizeof (struct ocp_recovery_device_hw_hsp_sp1_state));

	return 0;
}

/**
 * Release the resources used for an HSP SP1 OCP recovery handler.
 *
 * @param recovery The recovery handler to release.
 */
void ocp_recovery_device_hw_hsp_sp1_release (const struct ocp_recovery_device_hw_hsp_sp1 *recovery)
{
	UNUSED (recovery);
}

/**
 * Reset the flag that indicates a recovery image was loaded into memory and validated.
 *
 * @param recovery The recovery handler to update.
 */
void ocp_recovery_device_hw_hsp_sp1_clear_image_loaded (
	const struct ocp_recovery_device_hw_hsp_sp1 *recovery)
{
	if (recovery) {
		recovery->base.state->image_loaded = false;
	}
}

/**
 * Determine if a recovery image has been loaded into memory and validated.
 *
 * @param recovery The recovery handler to query.
 *
 * @return true if there is a validated image in memory ready to execute or false if not.
 */
bool ocp_recovery_device_hw_hsp_sp1_is_image_loaded (
	const struct ocp_recovery_device_hw_hsp_sp1 *recovery)
{
	if (recovery) {
		return recovery->base.state->image_loaded;
	}
	else {
		return false;
	}
}
