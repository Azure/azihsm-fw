// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef BOOT_MEASUREMENTS_SINGLE_ROOT_H_
#define BOOT_MEASUREMENTS_SINGLE_ROOT_H_

#include <stdint.h>
#include "boot_measurements.h"
#include "crypto/hash.h"
#include "drivers/ccs_ksu_interface.h"
#include "drivers/fuse_controller_interface.h"
#include "firmware/hsp_fw_1sp.h"
#include "firmware/hw_rot.h"
#include "firmware/key_manifest_hsp_rom.h"
#include "splibs/inc/spcryptotypes.h"
#include "status/msft_module_id.h"


#pragma pack(push, 1)

/**
 * Log of boot measurements for single root key HSP devices.
 */
struct boot_measurements_single_root_log {
	struct boot_measurements_security_state security_state;		/**< Security state event data and digest. */
	struct boot_measurements_public_key owner_public_key;		/**< Owner public key event data and digest. */
	struct boot_measurements_svn key_manifest_svn;				/**< Key manifest SVN event data and digest. */
	struct boot_measurements_tenancy_counter tenancy_counter;	/**< Tenancy counter event data and digest. */
	struct boot_measurements_public_key fw_public_key;			/**< Firmware public key event data and digest. */
	struct boot_measurements_public_key secondary_public_key;	/**< Secondary or Tenancy Grant public key event data and digest. */
	struct boot_measurements_svn fw_svn;						/**< Firmware SVN event data and digest. */
	struct boot_measurements_build_version fw_version;			/**< Firmware build version event data and digest. */
	struct boot_measurements_digest fw_image;					/**< Firmware image event data and digest. */
};

#pragma pack(pop)


int boot_measurements_single_root_generate_log (struct boot_measurements_single_root_log *log,
	const struct hash_engine *hash, const struct fuse_controller_interface *fuses,
	uint8_t a0_bypass, uint32_t reset_type,
	const uint8_t rng_cal[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH], const struct hw_rot *rot,
	const struct key_manifest_hsp_rom *keys, const struct hsp_fw_1sp *fw);

int boot_measurements_single_root_extend_pcr (const struct boot_measurements_single_root_log *log,
	const struct ccs_ksu_interface *ccs, uint8_t pcr);

int boot_measurements_single_root_generate_device_independent_fwid (
	const struct boot_measurements_single_root_log *log, const struct hash_engine *hash,
	SP_MSG_384 *fwid);


#endif	/* BOOT_MEASUREMENTS_SINGLE_ROOT_H_ */
