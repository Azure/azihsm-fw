// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OCP_RECOVERY_DEVICE_HW_HSP_SP1_STATIC_H_
#define OCP_RECOVERY_DEVICE_HW_HSP_SP1_STATIC_H_

#include "ocp_recovery_device_hw_hsp_rom_static.h"
#include "ocp_recovery_device_hw_hsp_sp1.h"

/* Internal functions declared to allow for static initialization. */
void ocp_recovery_device_hw_hsp_sp1_get_device_status (
	const struct ocp_recovery_device_hw *recovery_hw,
	enum ocp_recovery_device_status_code *status_code,
	enum ocp_recovery_recovery_reason_code *reason_code,
	struct ocp_recovery_device_status_vendor *vendor);
int ocp_recovery_device_hw_hsp_sp1_activate_recovery (
	const struct ocp_recovery_device_hw *recovery_hw,
	const struct ocp_recovery_device_cms *recovery, bool *is_auth_error);

/**
 * Constant initializer for the recovery handler API with full feature support.
 */
#define	OCP_RECOVERY_DEVICE_HW_HSP_SP1_RECOVERY_INIT	{ \
		.get_device_id = ocp_recovery_device_hw_hsp_rom_get_device_id, \
		.get_device_status = ocp_recovery_device_hw_hsp_sp1_get_device_status, \
		.reset_device = NULL, \
		.reset_management = ocp_recovery_device_hw_hsp_rom_reset_management, \
		.activate_recovery = ocp_recovery_device_hw_hsp_sp1_activate_recovery, \
		.supports_forced_recovery = true \
	}

/**
 * Initialize a static instance of an HSP SP1 OCP recovery handler.  This does not initialize the
 * handler state.  That will need to be initialized separately with
 * ocp_recovery_device_hw_hsp_sp1_init_state.
 *
 * There is no validation done on the arguments.
 *
 * @param api The API initialization block to use for the handler.
 * @param rom_state_ptr Variable context for the HSP ROM handler.
 * @param rom_manifest_ptr Key manifest handler for parsing manifests on recovery images.
 * @param pka_ptr The PKA engine to use for signature verification.
 * @param hash_ptr The hash engine to use for image verification.
 * @param socid_ptr The SOCID for the device.
 * @param reset_callback Callback function to use to reset the device.
 * @param sp1_state_ptr Variable context for the HSP SP1 handler.
 */
#define	ocp_recovery_device_hw_hsp_sp1_static_init(api, rom_state_ptr, rom_manifest_ptr, \
	pka_ptr, hash_ptr, socid_ptr, reset_callback, sp1_state_ptr)	{ \
		.base = { \
			.base = api, \
			.state = rom_state_ptr, \
			.manifest = rom_manifest_ptr, \
			.image = NULL, \
			.pka = pka_ptr, \
			.hash = hash_ptr, \
			.socid = socid_ptr, \
			.reset = reset_callback \
		}, \
		.state = sp1_state_ptr \
	}


#endif	/* OCP_RECOVERY_DEVICE_HW_HSP_SP1_STATIC_H_ */
