// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef BOOT_MEASUREMENTS_DUAL_ROOT_H_
#define BOOT_MEASUREMENTS_DUAL_ROOT_H_

#include <stdint.h>
#include "boot_measurements.h"
#include "drivers/ccs_ksu_interface.h"
#include "firmware/key_manifest_hsp_rom_dual_root.h"


#pragma pack(push, 1)

/**
 * Log of boot measurements for the device when using dual root key manifests.
 */
struct boot_measurements_dual_root_log {
	struct boot_measurements_security_state security_state;		/**< Security state event dat and digesta. */
	struct boot_measurements_public_key owner_public_key;		/**< Owner public key event data and digest. */
	struct boot_measurements_public_key authority_public_key;	/**< Authority public key event data and digest. */
	struct boot_measurements_svn key_manifest_svn;				/**< Key manifest SVN event data and digest. */
	struct boot_measurements_tenancy_counter tenancy_counter;	/**< Tenancy counter event data and digest. */
	struct boot_measurements_public_key fw_public_key;			/**< Firmware public key event data and digest. */
	struct boot_measurements_public_key secondary_public_key;	/**< Secondary public key event data and digest. */
	struct boot_measurements_public_key grant_public_key;		/**< Tenancy Grant public key event data and digest. */
	struct boot_measurements_svn fw_svn;						/**< Firmware SVN event data and digest. */
	struct boot_measurements_build_version fw_version;			/**< Firmware build version event data and digest. */
	struct boot_measurements_digest fw_image;					/**< Firmware image event data and digest. */
};

#pragma pack(pop)


int boot_measurements_dual_root_generate_log (struct boot_measurements_dual_root_log *log,
	const struct hash_engine *hash, const struct fuse_controller_interface *fuses,
	uint8_t a0_bypass, uint32_t reset_type,
	const uint8_t rng_cal[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH], const struct hw_rot *rot,
	const struct key_manifest_hsp_rom_dual_root *keys, const struct hsp_fw_1sp *fw);

int boot_measurements_dual_root_extend_pcr (const struct boot_measurements_dual_root_log *log,
	const struct ccs_ksu_interface *ccs, uint8_t pcr);


#endif	/* BOOT_MEASUREMENTS_DUAL_ROOT_H_ */
