// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_FIRMWARE_DESCRIPTOR_H_
#define MANTICORE_FIRMWARE_DESCRIPTOR_H_

#include <stdint.h>
#include "crypto/hash.h"
#include "crypto/signature_verification.h"
#include "firmware/firmware_component.h"
#include "firmware/firmware_header.h"
#include "flash/flash.h"
#include "status/manticore_module_id.h"


/**
 * Specify a maximum length for a compatible firmware descriptor.
 */
#define	MANTICORE_FIRMWARE_DESCRIPTOR_MAX_LENGTH		256


#pragma pack(push, 1)
/**
 * Minimum contents of the firmware descriptor.  This represents format version 0.
 */
struct manticore_firmware_descriptor_data_v0 {
	uint64_t svn;			/**< Security Version Number for the firmware package. */
	uint32_t reset_vector;	/**< Reset vector for the run-time SP firmware. */
	uint8_t sp_count;		/**< Number of SP firmware components. */
	uint8_t cp_count;		/**< Number of CP firmware components. */
	uint8_t fp0_count;		/**< Number of FP firmware components for core 0. */
	uint8_t fp1_count;		/**< Number of FP firmware components for core 1. */
	uint8_t fp2_count;		/**< Number of FP firmware components for core 2. */
};

/**
 * Contents of firmware descriptor format version 1.
 */
struct manticore_firmware_descriptor_data_v1 {
	uint64_t svn;				/**< Security Version Number for the firmware package. */
	uint32_t reset_vector;		/**< Reset vector for the run-time SP firmware. */
	uint8_t sp_count;			/**< Number of SP firmware components. */
	uint8_t cp_count;			/**< Number of CP firmware components. */
	uint8_t fp0_count;			/**< Number of FP firmware components for core 0. */
	uint8_t fp1_count;			/**< Number of FP firmware components for core 1. */
	uint8_t fp2_count;			/**< Number of FP firmware components for core 2. */
	uint8_t pcie_count;			/**< Number of PCIe PHY firmware components. */
	uint16_t compat_version;	/**< Impactless firmware update compatibility version. */
};

/**
 * Contents of the firmware descriptor format version 2.
 */
struct manticore_firmware_descriptor_data_v2 {
	uint64_t svn;				/**< Security Version Number for the firmware package. */
	uint32_t reset_vector;		/**< Reset vector for the run-time SP firmware. */
	uint8_t sp_count;			/**< Number of SP firmware components. */
	uint8_t cp_count;			/**< Number of CP firmware components. */
	uint8_t fp0_count;			/**< Number of FP firmware components for core 0. */
	uint8_t fp1_count;			/**< Number of FP firmware components for core 1. */
	uint8_t fp2_count;			/**< Number of FP firmware components for core 2. */
	uint8_t pcie_count;			/**< Number of PCIe PHY firmware components. */
	uint16_t compat_version;	/**< Impactless firmware update compatibility version. */
	uint8_t fips_certified;		/**< Indication that the firmware has a FIPS certification. */
};

/**
 * Contents of the firmware descriptor.  This represents format version 3.
 */
struct manticore_firmware_descriptor_data {
	uint64_t svn;				/**< Security Version Number for the firmware package. */
	uint32_t reset_vector;		/**< Reset vector for the run-time SP firmware. */
	uint8_t sp_count;			/**< Number of SP firmware components. */
	uint8_t cp_count;			/**< Number of CP firmware components. */
	uint8_t fp0_count;			/**< Number of FP firmware components for core 0. */
	uint8_t fp1_count;			/**< Number of FP firmware components for core 1. */
	uint8_t fp2_count;			/**< Number of FP firmware components for core 2. */
	uint8_t pcie_count;			/**< Number of PCIe PHY firmware components. */
	uint16_t compat_version;	/**< Impactless firmware update compatibility version. */
	uint8_t fips_certified;		/**< Indication that the firmware has a FIPS certification. */
	uint8_t bks_isolation;		/**< Indication that BKS should be different between FIPS and Non-FIPS. */
};

#pragma pack(pop)

_Static_assert ((sizeof (struct manticore_firmware_descriptor_data) <=
	MANTICORE_FIRMWARE_DESCRIPTOR_MAX_LENGTH), "FW descriptor too large.");

/**
 * Parser for the Firmware Descriptor on a Manticore Firmware Package.
 */
struct manticore_firmware_descriptor {
	union {
		struct manticore_firmware_descriptor_data data;				/**< Data contained in the descriptor. */
		uint8_t max_data[MANTICORE_FIRMWARE_DESCRIPTOR_MAX_LENGTH];	/**< Maximum length allowed for a descriptor. */
	};

	uint8_t build_ver[FW_COMPONENT_BUILD_VERSION_LENGTH];			/**< The Build Version Number for the firmware package. */
	size_t length;													/**< The total length of the descriptor component. */
	int recovery_rev;												/**< The recovery revision from the firmware header. */
	int allowed_rev;												/**< The earliest allow revision from the firmware header. */
	size_t data_length;												/**< Length of the data in the descriptor. */

	/**
	 * Reserved for future expansion to maintain compatibility between builds.  When adding fields
	 * to this structure, the reserved size should be appropriately reduced to maintain a constant
	 * size when built for RISC-V.
	 */
	uint8_t reserved[60];
};


/**
 * Expected constant size of the parsed descriptor structure when built for 32-bit RISC-V.
 */
#define	MANTICORE_FIRMWARE_DESCRIPTOR_STRUCT_SIZE		MANTICORE_FIRMWARE_DESCRIPTOR_MAX_LENGTH + \
	FW_COMPONENT_BUILD_VERSION_LENGTH + 76


int manticore_firmware_descriptor_init (struct manticore_firmware_descriptor *fw_descriptor,
	struct firmware_header *fw_header, const struct flash *flash, uint32_t address,
	const struct hash_engine *hash, const struct signature_verification *verification);

size_t manticore_firmware_descriptor_get_component_length (
	const struct manticore_firmware_descriptor *fw);
size_t manticore_firmware_descriptor_get_data_length (
	const struct manticore_firmware_descriptor *fw);

uint64_t manticore_firmware_descriptor_get_svn (const struct manticore_firmware_descriptor *fw);
int manticore_firmware_descriptor_get_recovery_revision (
	const struct manticore_firmware_descriptor *fw);
int manticore_firmware_descriptor_get_earliest_allowed_revision (
	const struct manticore_firmware_descriptor *fw);

const uint8_t* manticore_firmware_descriptor_get_build_version (
	const struct manticore_firmware_descriptor *fw);

uint8_t manticore_firmware_descriptor_sp_image_count (
	const struct manticore_firmware_descriptor *fw);
uint32_t manticore_firmware_descriptor_sp_reset_vector (
	const struct manticore_firmware_descriptor *fw);

uint8_t manticore_firmware_descriptor_cp_image_count (
	const struct manticore_firmware_descriptor *fw);

uint16_t manticore_firmware_descriptor_fp_image_count (
	const struct manticore_firmware_descriptor *fw);
uint8_t manticore_firmware_descriptor_fp0_image_count (
	const struct manticore_firmware_descriptor *fw);
uint8_t manticore_firmware_descriptor_fp1_image_count (
	const struct manticore_firmware_descriptor *fw);
uint8_t manticore_firmware_descriptor_fp2_image_count (
	const struct manticore_firmware_descriptor *fw);

uint8_t manticore_firmware_descriptor_pcie_image_count (
	const struct manticore_firmware_descriptor *fw);

uint16_t manticore_firmware_descriptor_image_compatibility_version (
	const struct manticore_firmware_descriptor *fw);

uint8_t manticore_firmware_descriptor_fips_certified (
	const struct manticore_firmware_descriptor *fw);

uint8_t manticore_firmware_descriptor_bks_fips_isolation (
	const struct manticore_firmware_descriptor *fw);


#define	MANTICORE_FW_DESCRIPTOR_ERROR(code)     \
	ROT_ERROR (MANTICORE_MODULE_MANTICORE_FW_DESCRIPTOR, code)

/**
 * Error codes that can be generated by the firmware descriptor parser.
 */
enum {
	MANTICORE_FW_DESCRIPTOR_INVALID_ARGUMENT = MANTICORE_FW_DESCRIPTOR_ERROR (0x00),	/**< Input parameter is null or not valid. */
	MANTICORE_FW_DESCRIPTOR_NO_MEMORY = MANTICORE_FW_DESCRIPTOR_ERROR (0x01),			/**< Memory allocation failed. */
	MANTICORE_FW_DESCRIPTOR_NO_BUILD_VERSION = MANTICORE_FW_DESCRIPTOR_ERROR (0x02),	/**< The firmware descriptor does not provide a build version. */
	MANTICORE_FW_DESCRIPTOR_OLD_FW_HEADER = MANTICORE_FW_DESCRIPTOR_ERROR (0x03),		/**< The firmware header uses an incompatible format. */
	MANTICORE_FW_DESCRIPTOR_NO_PKG_SIGNATURE = MANTICORE_FW_DESCRIPTOR_ERROR (0x04),	/**< The firmware header indicates there is no signature on the entire package. */
	MANTICORE_FW_DESCRIPTOR_INCONSISTENT = MANTICORE_FW_DESCRIPTOR_ERROR (0x05),		/**< The firmware header and descriptor data do not match. */
	MANTICORE_FW_DESCRIPTOR_TOO_SHORT = MANTICORE_FW_DESCRIPTOR_ERROR (0x06),			/**< The descriptor data is less than the minimum required length. */
	MANTICORE_FW_DESCRIPTOR_TOO_LARGE = MANTICORE_FW_DESCRIPTOR_ERROR (0x07),			/**< The descriptor data is more than the maximum allowed length. */
	MANTICORE_FW_DESCRIPTOR_BAD_FORMAT = MANTICORE_FW_DESCRIPTOR_ERROR (0x08),			/**< The descriptor data does not follow a known format.  */
};


#endif	/* MANTICORE_FIRMWARE_DESCRIPTOR_H_ */
