// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OCP_RECOVERY_DEVICE_HW_HSP_ROM_H_
#define OCP_RECOVERY_DEVICE_HW_HSP_ROM_H_

#include <stdbool.h>
#include "crypto/ecc_hw.h"
#include "crypto/hash.h"
#include "firmware/hsp_fw_1sp.h"
#include "firmware/key_manifest_hsp_rom.h"
#include "recovery/ocp_recovery_device.h"


/**
 * Variable context for the HSP ROM recovery handler.
 */
struct ocp_recovery_device_hw_hsp_rom_state {
	bool image_loaded;	/**< Flag indicating if a recovery image was successfully loaded. */
};

/**
 * Handler for OCP Recovery actions in HSP ROM.
 */
struct ocp_recovery_device_hw_hsp_rom {
	struct ocp_recovery_device_hw base;					/**< The base recovery interface. */
	struct ocp_recovery_device_hw_hsp_rom_state *state;	/**< Variable handler context. */
	const struct key_manifest_hsp_rom *manifest;		/**< Recovery key manifest handler. */
	const struct hsp_fw_1sp *image;						/**< Recovery 1SP image handler. */
	const struct ecc_hw *pka;							/**< PKA engine for verification. */
	const struct hash_engine *hash;						/**< Hash engine for verification. */
	const uint32_t *socid;								/**< SOCID for the device. */

	/**
	 * Callback function to provide a platform-specific method for resetting the device from the
	 * recovery interface.
	 *
	 * @param force_recovery True to force recovery mode after the reset.
	 */
	void (*reset) (bool force_recovery);
};


int ocp_recovery_device_hw_hsp_rom_init (struct ocp_recovery_device_hw_hsp_rom *recovery,
	struct ocp_recovery_device_hw_hsp_rom_state *state, const struct key_manifest_hsp_rom *manifest,
	const struct hsp_fw_1sp *image, const struct ecc_hw *pka, const struct hash_engine *hash,
	const uint32_t *socid, void (*reset) (bool), bool force_recovery);
int ocp_recovery_device_hw_hsp_rom_init_no_recovery (
	struct ocp_recovery_device_hw_hsp_rom *recovery,
	struct ocp_recovery_device_hw_hsp_rom_state *state, const uint32_t *socid, void (*reset) (bool),
	bool force_recovery);
int ocp_recovery_device_hw_hsp_rom_init_state (
	const struct ocp_recovery_device_hw_hsp_rom *recovery);
void ocp_recovery_device_hw_hsp_rom_release (const struct ocp_recovery_device_hw_hsp_rom *recovery);

void ocp_recovery_device_hw_hsp_rom_clear_image_loaded (
	const struct ocp_recovery_device_hw_hsp_rom *recovery);
bool ocp_recovery_device_hw_hsp_rom_is_image_loaded (
	const struct ocp_recovery_device_hw_hsp_rom *recovery);


#endif	/* OCP_RECOVERY_DEVICE_HW_HSP_ROM_H_ */
