// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OCP_RECOVERY_DEVICE_HW_HSP_SP1_H_
#define OCP_RECOVERY_DEVICE_HW_HSP_SP1_H_

#include <stdbool.h>
#include "crypto/ecc_hw.h"
#include "crypto/hash.h"
#include "recovery/ocp_recovery_device.h"
#include "recovery/ocp_recovery_device_hw_hsp_rom.h"

/**
 * Variable context for the HSP SP1 recovery handler.
 */
struct ocp_recovery_device_hw_hsp_sp1_state {
	struct ocp_recovery_device_hw_hsp_rom_state rom_state;	/**< Variable context for the HSP ROM recovery handler. */
	uint8_t last_fail_id;									/**< Cache for the last recovery error. */
	uint32_t last_error_code;								/**< Cache for the detailed error code of the last recovery error. */
};

/**
 * Handler for OCP Recovery actions in HSP SP1 Recovery.
 */
struct ocp_recovery_device_hw_hsp_sp1 {
	struct ocp_recovery_device_hw_hsp_rom base;			/**< The base recovery interface. */
	struct ocp_recovery_device_hw_hsp_sp1_state *state;	/**< Variable handler context. */
};


int ocp_recovery_device_hw_hsp_sp1_init (struct ocp_recovery_device_hw_hsp_sp1 *recovery,
	struct ocp_recovery_device_hw_hsp_rom_state *rom_state,
	const struct key_manifest_hsp_rom *manifest, const struct ecc_hw *pka,
	const struct hash_engine *hash,	const uint32_t *socid, void (*reset) (bool),
	struct ocp_recovery_device_hw_hsp_sp1_state *sp1_state);
int ocp_recovery_device_hw_hsp_sp1_init_state (
	const struct ocp_recovery_device_hw_hsp_sp1 *recovery);
void ocp_recovery_device_hw_hsp_sp1_release (const struct ocp_recovery_device_hw_hsp_sp1 *recovery);

void ocp_recovery_device_hw_hsp_sp1_clear_image_loaded (
	const struct ocp_recovery_device_hw_hsp_sp1 *recovery);
bool ocp_recovery_device_hw_hsp_sp1_is_image_loaded (
	const struct ocp_recovery_device_hw_hsp_sp1 *recovery);


#endif	/* OCP_RECOVERY_DEVICE_HW_HSP_SP1_H_ */
