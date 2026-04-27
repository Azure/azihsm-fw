// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef BOOT_MEASUREMENTS_H_
#define BOOT_MEASUREMENTS_H_

#include <stdint.h>
#include "crypto/hash.h"
#include "drivers/fuse_controller_interface.h"
#include "firmware/hsp_fw_1sp.h"
#include "firmware/hw_rot.h"
#include "firmware/key_manifest_hsp_rom.h"
#include "splibs/inc/spcryptotypes.h"
#include "status/msft_module_id.h"


/* Configurable boot measurements parameters.  Defaults can be overridden in platform_config.h. */
#include "platform_config.h"
#ifndef BOOT_MEASURMENTS_TENANCY_COUNTER_LENGTH
#define	BOOT_MEASURMENTS_TENANCY_COUNTER_LENGTH			(256u)
#endif

#if BOOT_MEASURMENTS_TENANCY_COUNTER_LENGTH & 0x3
#error "The tenancy counter length must align to 32-bit words."
#endif

/**
 * Event IDs for the data that is measured as part of firmware loading.
 */
enum {
	BOOT_MEASUREMENTS_EVENT_SECURITY_STATE = 0xe1000000,				/**< Device life-cycle security configuration .*/
	BOOT_MEASUREMENTS_EVENT_OWNER_PUBLIC_KEY = 0xe1000001,				/**< Root key used to verify the key manifest. */
	BOOT_MEASUREMENTS_EVENT_KEY_MANIFEST_SVN = 0xe1000002,				/**< SVN for the key manifest used to verify the firmware image. */
	BOOT_MEASUREMENTS_EVENT_TENANCY_COUNTER = 0xe1000003,				/**< Value of the tenancy counter. */
	BOOT_MEASUREMENTS_EVENT_FW_PUBLIC_KEY = 0xe1000004,					/**< Public key used to verify the firmware image. */
	BOOT_MEASUREMENTS_EVENT_SECONDARY_PUBLIC_KEY = 0xe1000005,			/**< Secondary public key used to verify the firmware image. */
	BOOT_MEASUREMENTS_EVENT_TENANCY_GRANT_KEY = 0xe1000006,				/**< Tenancy grant public key used to verify the grant manifest. */
	BOOT_MEASUREMENTS_EVENT_FW_SVN = 0xe1000007,						/**< SVN for the firmware image. */
	BOOT_MEASUREMENTS_EVENT_BUILD_VERSION = 0xe1000008,					/**< Build version number for the firmware image. */
	BOOT_MEASUREMENTS_EVENT_FW_IMAGE = 0xe1000009,						/**< Digest of the firmware binary that was loaded. */
	BOOT_MEASUREMENTS_EVENT_SECONDARY_OWNER_PUBLIC_KEY = 0xe100000a,	/**< Digest of the secondary root key used to verify the manifest. */
};


#pragma pack(push, 1)
/**
 * Header that is prepended to all data to identify the type of data that is being measured.
 */
struct boot_measurements_header {
	uint32_t event_id;	/**< ID of the measured event. */
	uint8_t version;	/**< Version identifier of the event data structure. */
};

/**
 * Base security state of the device.
 */
struct boot_measurments_security_state_data {
	struct boot_measurements_header event;								/**< Event information. */
	uint8_t security_state;												/**< The current lifecycle security state. */
	uint8_t rng_calibration[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH];	/**< Calibration data applied to the HW RNG. */
	uint8_t a0_bypass;													/**< State of the A0 bypass signal. */
	uint32_t reset_type;												/**< Indication of the what caused the device reset. */
};

/**
 * Security state measurement log entry.
 */
struct boot_measurements_security_state {
	struct boot_measurments_security_state_data data;	/**< Security state data. */
	SP_MSG_384 digest;									/**< Measurement of the security state data. */
};

/**
 * Public key from the image key manifest.
 */
struct boot_measurements_public_key_data {
	struct boot_measurements_header event;	/**< Event information. */
	SP_ECDSA_P384_PUBLIC key;				/**< Public key used for verification. */
};

/**
 * Public key measurement log entry.
 */
struct boot_measurements_public_key {
	struct boot_measurements_public_key_data data;	/**< Public key data. */
	SP_MSG_384 digest;								/**< Measurement of the public key data. */
};

/**
 * Security Version Number for an image component.
 */
struct boot_measurements_svn_data {
	struct boot_measurements_header event;	/**< Event information. */
	uint32_t svn;							/**< SVN value for the image. */
};

/**
 * SVN measurement log entry.
 */
struct boot_measurements_svn {
	struct boot_measurements_svn_data data;	/**< SVN data. */
	SP_MSG_384 digest;						/**< Measurement of the SVN data. */
};

/**
 * Value of the tenancy counter for the loaded firmware image.
 */
struct boot_measurements_tenancy_counter_data {
	struct boot_measurements_header event;						/**< Event information. */
	uint8_t counter[BOOT_MEASURMENTS_TENANCY_COUNTER_LENGTH];	/**< Value of the tenancy counter. */
};

/**
 * Tenancy counter measurement log entry.
 */
struct boot_measurements_tenancy_counter {
	struct boot_measurements_tenancy_counter_data data;	/**< Tenancy data. */
	SP_MSG_384 digest;									/**< Measurement of the Tenancy data. */
};

/**
 * The build version number for the loaded firmware image.
 */
struct boot_measurements_build_version_data {
	struct boot_measurements_header event;			/**< Event information. */
	uint8_t build_version[HSP_FW_1SP_VERSION_LEN];	/**< Version identifier for the loaded image. */
};

/**
 * Firmware version measurement log entry.
 */
struct boot_measurements_build_version {
	struct boot_measurements_build_version_data data;	/**< FW version data. */
	SP_MSG_384 digest;									/**< Measurement of the FW version data. */
};

/**
 * Digest of a loaded binary image.
 */
struct boot_measurements_digest_data {
	struct boot_measurements_header event;	/**< Event information. */
	SP_MSG_384 digest;						/**< Digest of the binary image. */
};

/**
 * Firmware image digest measurement log entry.
 */
struct boot_measurements_digest {
	struct boot_measurements_digest_data data;	/**< FW image digest data. */
	SP_MSG_384 digest;							/**< Measurement of the FW image digest data. */
};

#pragma pack(pop)


int boot_measurements_generate_log_security_state (
	struct boot_measurements_security_state *state, const struct hash_engine *hash,
	const struct fuse_controller_interface *fuses, uint8_t a0_bypass, uint32_t reset_type,
	const uint8_t rng_cal[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH]);
int boot_measurements_generate_log_public_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct key_manifest_public_key *key, uint32_t event);
int boot_measurements_generate_log_primary_root_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct hw_rot *rot, const struct key_manifest_hsp_rom *keys, int *hw_key_out);
int boot_measurements_generate_log_primary_firmware_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct key_manifest_hsp_rom *keys);
int boot_measurements_generate_log_secondary_firmware_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct key_manifest_hsp_rom *keys);
int boot_measurements_generate_log_tenancy_grant_key (
	struct boot_measurements_public_key *key_out, const struct hash_engine *hash,
	const struct key_manifest_hsp_rom *keys, bool required);
int boot_measurements_generate_log_manifest_svn (struct boot_measurements_svn *svn,
	const struct hash_engine *hash, const struct key_manifest_hsp_rom *keys, int hw_key);
int boot_measurements_generate_log_manifest_tenancy_counter (
	struct boot_measurements_tenancy_counter *tenancy, const struct hash_engine *hash,
	const struct hw_rot *rot);
int boot_measurements_generate_log_firmware_svn (struct boot_measurements_svn *svn,
	const struct hash_engine *hash, const struct hsp_fw_1sp *fw);
int boot_measurements_generate_log_firmware_version (
	struct boot_measurements_build_version *fw_version, const struct hash_engine *hash,
	const struct hsp_fw_1sp *fw);
int boot_measurements_generate_log_firmware_image (struct boot_measurements_digest *fw_image,
	const struct hash_engine *hash, const struct hsp_fw_1sp *fw);


#define	BOOT_MEASUREMENTS_ERROR(code)		ROT_ERROR (MSFT_MODULE_BOOT_MEASUREMENTS, code)

/**
 * Error codes that can be generated while generating the boot measurements.
 */
enum {
	BOOT_MEASUREMENTS_INVALID_ARGUMENT = BOOT_MEASUREMENTS_ERROR (0x00),	/**< Input parameter is null or not valid. */
	BOOT_MEASUREMENTS_NO_MEMORY = BOOT_MEASUREMENTS_ERROR (0x01),			/**< Memory allocation failed. */
	BOOT_MEASUREMENTS_NO_ROOT_KEY = BOOT_MEASUREMENTS_ERROR (0x02),			/**< No root key was provided by the key manifest. */
	BOOT_MEASUREMENTS_NO_FW_KEY = BOOT_MEASUREMENTS_ERROR (0x03),			/**< No FW key was provided by the key manifest. */
	BOOT_MEASUREMENTS_NO_TENANCY_KEY = BOOT_MEASUREMENTS_ERROR (0x04),		/**< No tenancy grant key was provided by the key manifest. */
};


#endif	/* BOOT_MEASUREMENTS_H_ */
