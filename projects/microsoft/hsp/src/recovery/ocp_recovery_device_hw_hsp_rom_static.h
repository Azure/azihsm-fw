// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OCP_RECOVERY_DEVICE_HW_HSP_ROM_STATIC_H_
#define OCP_RECOVERY_DEVICE_HW_HSP_ROM_STATIC_H_

#include "ocp_recovery_device_hw_hsp_rom.h"


/* Internal functions declared to allow for static initialization. */
int ocp_recovery_device_hw_hsp_rom_get_device_id (const struct ocp_recovery_device_hw *recovery_hw,
	struct ocp_recovery_device_id *id);
void ocp_recovery_device_hw_hsp_rom_get_device_status (
	const struct ocp_recovery_device_hw *recovery_hw,
	enum ocp_recovery_device_status_code *status_code,
	enum ocp_recovery_recovery_reason_code *reason_code,
	struct ocp_recovery_device_status_vendor *vendor);
void ocp_recovery_device_hw_hsp_rom_reset_management (
	const struct ocp_recovery_device_hw *recovery_hw, bool forced_recovery);
int ocp_recovery_device_hw_hsp_rom_activate_recovery (
	const struct ocp_recovery_device_hw *recovery_hw,
	const struct ocp_recovery_device_cms *recovery, bool *is_auth_error);


/**
 * Constant initializer for the recovery handler API with full feature support.
 */
#define	OCP_RECOVERY_DEVICE_HW_HSP_ROM_RECOVERY_INIT	{ \
		.get_device_id = ocp_recovery_device_hw_hsp_rom_get_device_id, \
		.get_device_status = ocp_recovery_device_hw_hsp_rom_get_device_status, \
		.reset_device = NULL, \
		.reset_management = ocp_recovery_device_hw_hsp_rom_reset_management, \
		.activate_recovery = ocp_recovery_device_hw_hsp_rom_activate_recovery, \
		.supports_forced_recovery = true \
	}

/**
 * Constant initializer for the recovery handler API that does not support forced recovery.
 */
#define	OCP_RECOVERY_DEVICE_HW_HSP_ROM_RECOVERY_INIT_NO_FORCE	{ \
		.get_device_id = ocp_recovery_device_hw_hsp_rom_get_device_id, \
		.get_device_status = ocp_recovery_device_hw_hsp_rom_get_device_status, \
		.reset_device = NULL, \
		.reset_management = ocp_recovery_device_hw_hsp_rom_reset_management, \
		.activate_recovery = ocp_recovery_device_hw_hsp_rom_activate_recovery, \
		.supports_forced_recovery = false \
	}

/**
 * Constant initializer for the handler API in a mode that does not support recovery commands.
 */
#define	OCP_RECOVERY_DEVICE_HW_HSP_ROM_INIT_NO_RECOVERY	{ \
		.get_device_id = ocp_recovery_device_hw_hsp_rom_get_device_id, \
		.get_device_status = ocp_recovery_device_hw_hsp_rom_get_device_status, \
		.reset_device = NULL, \
		.reset_management = ocp_recovery_device_hw_hsp_rom_reset_management, \
		.activate_recovery = NULL, \
		.supports_forced_recovery = true \
	}

/**
 * Constant initializer for the handler API in a mode that does not support recovery commands or
 * force recovery mode on reset.
 */
#define	OCP_RECOVERY_DEVICE_HW_HSP_ROM_INIT_NO_RECOVERY_NO_FORCE	{ \
		.get_device_id = ocp_recovery_device_hw_hsp_rom_get_device_id, \
		.get_device_status = ocp_recovery_device_hw_hsp_rom_get_device_status, \
		.reset_device = NULL, \
		.reset_management = ocp_recovery_device_hw_hsp_rom_reset_management, \
		.activate_recovery = NULL, \
		.supports_forced_recovery = false \
	}


/**
 * Initialize a static instance of an HSP ROM OCP recovery handler.  This does not initialize the
 * handler state.  That will need to be initialized separately with
 * ocp_recovery_device_hw_hsp_rom_init_state.
 *
 * There is no validation done on the arguments.
 *
 * @param api The API initialization block to use for the handler.
 * @param state_ptr Variable context for the HSP ROM handler.
 * @param manifest_ptr Key manifest handler for parsing manifests on recovery images.  This can be a
 * constant instance.
 * @param img_ptr 1SP firmware image handler for loading and verifying a recovery image.  This can
 * be a constant instance.
 * @param pka The PKA engine to use for signature verification.  This can be a constant instance.
 * @param hash The hash engine to use for image verification.  This can be a constant instance.
 * @param socid_ptr The SOCID for the device.
 * @param reset_callback Callback function to use to reset the device.
 */
#define	ocp_recovery_device_hw_hsp_rom_static_init(api, state_ptr, manifest_ptr, img_ptr, pka_ptr, \
	hash_ptr, socid_ptr, reset_callback)	{ \
		.base = api, \
		.state = state_ptr, \
		.manifest = manifest_ptr, \
		.image = img_ptr, \
		.pka = pka_ptr, \
		.hash = hash_ptr, \
		.socid = socid_ptr, \
		.reset = reset_callback \
	}


#endif	/* OCP_RECOVERY_DEVICE_HW_HSP_ROM_STATIC_H_ */
