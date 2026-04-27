// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_MEASUREMENTS_H_
#define MANTICORE_MEASUREMENTS_H_

#include <stdbool.h>
#include <stdint.h>
#include "drivers/ccs_ksu_interface.h"
#include "drivers/hsp_aeb.h"
#include "firmware/key_manifest_hsp_firmware.h"
#include "firmware/manticore_firmware_descriptor.h"
#include "rom/boot_measurements_single_root.h"
#include "status/manticore_module_id.h"
#include "system/security_policy_hsp_manticore.h"


/**
 * Event IDs for the data that is measured as part of firmware package loading.
 */
enum {
	MANTICORE_MEASUREMENTS_EVENT_SECURITY_POLICY = 0xe1000100,				/**< The security policy applied to the device. */
	MANTICORE_MEASUREMENTS_EVENT_FW_KEY_MANIFEST = 0xe1000101,				/**< Digest of the firmware key manifest used to verify the firmware package. */
	MANTICORE_MEASUREMENTS_EVENT_FW_KEY_MANIFEST_SVN = 0xe1000102,			/**< SVN for the key manifest used to verify the firmware package. */
	MANTICORE_MEASUREMENTS_EVENT_FW_PACKAGE_PUBLIC_KEY = 0xe1000103,		/**< Public key used to verify the firmware package. */
	MANTICORE_MEASUREMENTS_EVENT_FW_PACKAGE_SVN = 0xe1000104,				/**< SVN for the firmware package containing the loaded images. */
	MANTICORE_MEASUREMENTS_EVENT_FW_PACKAGE_BUILD_VERSION = 0xe1000105,		/**< Build version number for the firmware package. */
	MANTICORE_MEASUREMENTS_EVENT_SPRT_IMAGE = 0xe1000106,					/**< Digest of the SPRT binary that was loaded. */
	MANTICORE_MEASUREMENTS_EVENT_AEB_STATE = 0xe1000107,					/**< The state of all AEBs when starting SPRT execution. */
	MANTICORE_MEASUREMENTS_EVENT_AEB_LOCKED = 0xe1000108,					/**< The list of AEBs locked from modification by SPRT. */
	MANTICORE_MEASUREMENTS_EVENT_CP_FW_KEY_MANIFEST = 0xe1000200,			/**< Digest of the firmware key manifest used to verify the firmware package. */
	MANTICORE_MEASUREMENTS_EVENT_CP_FW_KEY_MANIFEST_SVN = 0xe1000201,		/**< SVN for the key manifest used to verify the firmware package. */
	MANTICORE_MEASUREMENTS_EVENT_CP_FW_PACKAGE_PUBLIC_KEY = 0xe1000202,		/**< Public key used to verify the firmware package. */
	MANTICORE_MEASUREMENTS_EVENT_CP_FW_PACKAGE_SVN = 0xe1000203,			/**< SVN for the firmware package containing the loaded images. */
	MANTICORE_MEASUREMENTS_EVENT_CP_FW_PACKAGE_BUILD_VERSION = 0xe1000204,	/**< Build version number for the firmware package. */
	MANTICORE_MEASUREMENTS_EVENT_CP_IMAGE = 0xe1000205,						/**< Digest of the CP binary that was loaded. */
	MANTICORE_MEASUREMENTS_EVENT_FP0_IMAGE = 0xe1000206,					/**< Digest of the FP0 binary that was loaded. */
	MANTICORE_MEASUREMENTS_EVENT_FP1_IMAGE = 0xe1000207,					/**< Digest of the FP1 binary that was loaded. */
	MANTICORE_MEASUREMENTS_EVENT_FP2_IMAGE = 0xe1000208,					/**< Digest of the FP2 binary that was loaded. */
	MANTICORE_MEASUREMENTS_EVENT_PCIE_PHY_IMAGE = 0xe1000209,				/**< Digest of the PCIe PHY binary that was loaded. */
};


#pragma pack(push, 1)
/**
 * Details of the security policy applied to the device.
 */
struct manticore_measurements_security_policy_data {
	struct boot_measurements_header event;				/**< Event information. */
	struct security_policy_hsp_manticore_data policy;	/**< The security policy data. */
};

/**
 * Device security policy measurement log entry.
 */
struct manticore_measurements_security_policy {
	struct manticore_measurements_security_policy_data data;	/**< Security policy log entry data. */
	SP_MSG_384 digest;											/**< Measurement of the entry data. */
};

/**
 * Security Version Number for run-time firmware components.
 */
struct manticore_measurements_svn_data {
	struct boot_measurements_header event;	/**< Event information. */
	uint64_t svn;							/**< SVN value for the image. */
};

/**
 * Run-time firmware SVN measurement log entry.
 */
struct manticore_measurements_svn {
	struct manticore_measurements_svn_data data;	/**< Firmware SVN log entry data. */
	SP_MSG_384 digest;								/**< Measurement of the entry data. */
};

/**
 * State of device Access Enablement Bits.
 */
struct manticore_measurements_aeb_data {
	struct boot_measurements_header event;	/**< Event information. */
	uint32_t aeb[4];						/**< The state for each AEB. */
};

/**
 * AEB configuration measurement log entry.
 */
struct manticore_measurements_aeb {
	struct manticore_measurements_aeb_data data;	/**< AEB log entry data. */
	SP_MSG_384 digest;								/**< Measurement of the entry data. */
};

/**
 * Log of firmware measurements for the SP core.
 */
struct manticore_measurements_log_sp {
	struct manticore_measurements_security_policy security_policy;	/**< Security policy log entry. */
	struct boot_measurements_digest fw_key_manifest;				/**< Firmware key manifest log entry. */
	struct manticore_measurements_svn fw_key_manifest_svn;			/**< Firmware key manifest SVN log entry. */
	struct boot_measurements_public_key fw_pkg_public_key;			/**< Firmware package public key log entry. */
	struct manticore_measurements_svn fw_pkg_svn;					/**< Firmware package SVN log entry. */
	struct boot_measurements_build_version fw_pkg_version;			/**< Firmware package build version log entry. */
	struct boot_measurements_digest sprt_image;						/**< SPRT image log entry. */
	struct manticore_measurements_aeb aeb_state;					/**< AEB state log entry. */
	struct manticore_measurements_aeb aeb_locked;					/**< AEB locked log entry. */
};

/**
 * Log of firmware measurements for the CP, FP, and PCIe PHY cores.
 */
struct manticore_measurements_log_soc {
	struct boot_measurements_digest fw_key_manifest;		/**< Firmware key manifest log entry. */
	struct manticore_measurements_svn fw_key_manifest_svn;	/**< Firmware key manifest SVN log entry. */
	struct boot_measurements_public_key fw_pkg_public_key;	/**< Firmware package public key log entry. */
	struct manticore_measurements_svn fw_pkg_svn;			/**< Firmware package SVN log entry. */
	struct boot_measurements_build_version fw_pkg_version;	/**< Firmware package build version log entry. */
	struct boot_measurements_digest cp_image;				/**< CP image log entry. */
	struct boot_measurements_digest fp0_image;				/**< FP core 0 image log entry. */
	struct boot_measurements_digest fp1_image;				/**< FP core 1 image log entry. */
	struct boot_measurements_digest fp2_image;				/**< FP core 2 image log entry. */
	struct boot_measurements_digest phy_image;				/**< PCIe PHY image log entry. */
};

/**
 * Complete log of measurements, including boot, SP, CP, FP, and PCIe PHY.
 */
struct manticore_measurements_log {
	struct boot_measurements_single_root_log rom;	/**< Measurements log from ROM. */
	struct manticore_measurements_log_sp sp;		/**< Measurements for the SP core. */
	struct manticore_measurements_log_soc soc;		/**< Measurements for the CP, FP, and PCIe PHY cores.*/
};

#pragma pack(pop)


int manticore_measurements_generate_sp_log (struct manticore_measurements_log_sp *log,
	const struct hash_engine *hash, const struct security_policy_hsp_manticore_data *policy,
	const struct key_manifest_hsp_firmware *fw_manifest,
	const struct manticore_firmware_descriptor *fw_descriptor, const SP_MSG_384 *sp_digest);
int manticore_measurements_update_sp_log_with_aeb_state (struct manticore_measurements_log_sp *log,
	const struct hash_engine *hash, const struct hsp_aeb *aeb);

int manticore_measurements_generate_soc_log (struct manticore_measurements_log_soc *log,
	const struct hash_engine *hash, const struct key_manifest_hsp_firmware *fw_manifest,
	const struct manticore_firmware_descriptor *fw_descriptor, const SP_MSG_384 *cp_digest,
	const SP_MSG_384 *fp0_digest, const SP_MSG_384 *fp1_digest, const SP_MSG_384 *fp2_digest,
	const SP_MSG_384 *phy_digest);

int manticore_measurements_extend_sp_pcr (const struct manticore_measurements_log_sp *log,
	const struct ccs_ksu_interface *ccs, uint8_t pcr);
int manticore_measurements_extend_sp_pcr_with_aeb_state (
	const struct manticore_measurements_log_sp *log, const struct ccs_ksu_interface *ccs,
	uint8_t pcr);

int manticore_measurements_verify_sp_pcr (const struct boot_measurements_single_root_log *rom_log,
	const struct manticore_measurements_log_sp *sp_log, const struct hash_engine *hash,
	const struct ccs_ksu_interface *ccs, uint8_t pcr, bool is_reinit);

int manticore_measurements_extend_soc_pcr (const struct manticore_measurements_log_soc *log,
	const struct ccs_ksu_interface *ccs, uint8_t pcr);
int manticore_measurements_verify_soc_pcr (const struct manticore_measurements_log_soc *log,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t pcr,
	bool is_reinit);

int manticore_measurements_generate_sprt_fwid (const struct manticore_measurements_log_sp *log,
	const struct hash_engine *hash, SP_MSG_384 *fwid);


#define	MANTICORE_MEASUREMENTS_ERROR(code)      \
	ROT_ERROR (MANTICORE_MODULE_MANTICORE_MEASUREMENTS, code)

/**
 * Error codes that can be generated while generating firmware measurements.
 */
enum {
	MANTICORE_MEASUREMENTS_INVALID_ARGUMENT = MANTICORE_MEASUREMENTS_ERROR (0x00),	/**< Input parameter is null or not valid. */
	MANTICORE_MEASUREMENTS_NO_MEMORY = MANTICORE_MEASUREMENTS_ERROR (0x01),			/**< Memory allocation failed. */
	MANTICORE_MEASUREMENTS_NO_FW_KEY = MANTICORE_MEASUREMENTS_ERROR (0x02),			/**< No firmware key in the key manifest. */
	MANTICORE_MEASUREMENTS_PCR_MISMATCH = MANTICORE_MEASUREMENTS_ERROR (0x03),		/**< The PCR does not match the log. */
};


#endif	/* MANTICORE_MEASUREMENTS_H_ */
