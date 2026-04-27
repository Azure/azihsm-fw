// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_ROM_H_
#define MANTICORE_ROM_H_

#include <stdlib.h>
#include "hsp_top.h"
#include "manticore_soc_rev.h"
#include "crypto/ecc_hw_pka.h"
#include "drivers/ccs_ksu.h"
#include "drivers/hs_sha.h"
#include "drivers/hsp_aes.h"
#include "drivers/hsp_fuses.h"
#include "firmware/hw_rot_hsp_rom.h"
#include "logging/debug_log.h"
#include "rom/boot_measurements_single_root.h"
#include "rom/device_keys.h"
#include "splibs/inc/spcryptotypes.h"


/**
 * The SoC-level GPIO register where status of boot strapping pins is stored.
 */
#define	MANTICORE_SOC_GPIO_STRAP				*((uint32_t*) 0xb0007104)

/**
 * Get the value of the Recovery Mode strapping pin.
 */
#define	MANTICORE_RECOVERY_MODE					(MANTICORE_SOC_GPIO_STRAP & (1U << 3))

/**
 * Get the value of the Flash Priority strapping pin.
 */
#define	MANTICORE_FLASH_PRIORITY				(MANTICORE_SOC_GPIO_STRAP & (1U << 4))


/**
 * Size of the shared SRAM buffer to use for hash operations.  This is the maximum amount of data
 * that can be hashed in a single operation.
 */
#define	MANTICORE_ROM_HS_SHA_BUFFER_SIZE		((4 * 1024) - 64)

/**
 * Size of the shard SRAM space that can be used for CCS HMAC operations.
 */
#define MANTICORE_ROM_CCS_HMAC_BUFFER_SIZE		\
	((sizeof (struct hs_sha_cmd_buffer) + MANTICORE_ROM_HS_SHA_BUFFER_SIZE) - \
		sizeof (struct ccs_cmd_buffer))

/**
 * Size of the buffer to use for storing ROM log messages.
 */
#define	MANTICORE_ROM_LOG_BUFFER_SIZE			(sizeof (struct debug_log_entry) * 32)

/**
 * The maximum number of ECC private keys supported by the CCS driver.
 */
#define	MANTICORE_ROM_MAX_ECC_PRIVATE_KEYS		3

/**
 * ROM partitioning for the HSP shared SRAM.  Some of this data is only used during ROM execution
 * and some of this is left in memory for firmware.
 */
struct manticore_rom_shared_sram {
	/**
	 * Data that is used internally during ROM execution.  The contents of this memory will be wiped
	 * before firmware begins executing.
	 *
	 * The command buffers for HW crypto blocks can overlap in the memory space since only one will
	 * be active at a time.
	 */
	union manticore_rom_shared_sram_internal {
		struct manticore_rom_shared_sram_ccs {
			struct ccs_cmd_buffer cmd;							/**< Command buffer for CCS HW. */
			uint8_t data[MANTICORE_ROM_CCS_HMAC_BUFFER_SIZE];	/**< Shared memory buffer available for CCS HMAC. */
		} ccs;													/**< Shared memory usage for CCS HW. */
		struct manticore_rom_shared_sram_hs_sha {
			struct hs_sha_cmd_buffer cmd;						/**< HS-SHA command buffer. */
			uint8_t data[MANTICORE_ROM_HS_SHA_BUFFER_SIZE];		/**< Temp data buffer for hash operations. */
		} hs_sha;												/**< Shared memory usage for HS-SHA HW. */
		struct manticore_rom_shared_sram_aes {
			struct hsp_aes_cmd_buffer cmd;						/**< AES command buffer. */
			uint8_t data[MANTICORE_ROM_HS_SHA_BUFFER_SIZE];		/**< Temp data buffer for AES operations. */
		} aes;													/**< Shared memory usage for AES HW. */
		struct ecc_hw_pka_cmd_buffer pka;						/**< Command buffer for PKA HW. */
	} internal;

	/**
	 * Data space that is both used internally during ROM execution and that contains information
	 * for firmware after ROM exits.
	 */
	union manticore_rom_shared_sram_overlap {
		/**
		 * Internal structures in the overlapping region of shared SRAM.
		 */
		struct manticore_rom_shared_sram_overlap_internal {
			struct hw_rot_hsp_rom_tenancy_buffer tenancy;		/**< Buffer for tenancy management. */
		} internal;

		/**
		 * Data left for firmware in the overlapping region of shared SRAM.
		 */
		struct manticore_rom_shared_sram_overlap_fw {
			uint8_t pad[sizeof (struct manticore_rom_shared_sram_overlap_internal) -
				HSP_FUSES_LENGTH (RSVD1)];						/**< Blank region of memory. */
			uint8_t rsvd1[HSP_FUSES_LENGTH (RSVD1)];			/**< Cache of the RSVD1 fuse data. */
		} firmware;
	} overlap;

	/**
	 * Data that that will be left intact by ROM for firmware use.
	 */
	struct manticore_rom_shared_sram_fw {
		uint8_t log_buffer[MANTICORE_ROM_LOG_BUFFER_SIZE];		/**< Data buffer for ROM log messages. */
		struct boot_measurements_single_root_log pcr_log[2];	/**< Log of measurement information for PCRs 0 and 1. */
		SP_ECDSA_P384_PUBLIC device_id_key;						/**< Device ID public key. */
		SP_ECDSA_P384_PUBLIC dme_key;							/**< DME public key. */
		SP_ECDSA_P384_PUBLIC dme_signing_key;					/**< Global key used to sign DME keys. */
		SP_ECDSA_P384_SIGNATURE dme_signature;					/**< CCS signature certifying the DME key. */
		struct device_keys_dice_endorsement dme_structure;		/**< DME structure endorsing the DICE identity key. */
	} firmware;
};

/* Make sure all the Shared SRAM is allocated. */
_Static_assert ((sizeof (struct manticore_rom_shared_sram) == sizeof (struct Sharedram_SHAREDRAM)),
	"ROM Shared SRAM partitioning does not match the available memory.");

/* Make sure the firmware shared data starts at the expected offset. */
_Static_assert ((offsetof (struct manticore_rom_shared_sram, overlap.firmware.rsvd1) == 0x1160),
	"Firmware data location in Shared SRAM does not start at the correct offset.");

/**
 * Define the maximum and minimum clock divider values allowed for RNG calibration.
 *
 * When building for simulation, allow any divider value.
 *
 * For other builds, do not let the divider be set below the default value and don't let it get set
 * so high that checkpoint timeouts will get triggered.
 */
#ifdef BUILD_FOR_SIMULATION
#define	MANTICORE_ROM_MIN_RNG_CLOCK_DIVIDER		0
#define	MANTICORE_ROM_MAX_RNG_CLOCK_DIVIDER		HSP_RNG_HW_MAX_CLOCK_DIVIDER
#else
#define	MANTICORE_ROM_MIN_RNG_CLOCK_DIVIDER		RNG_REGS_CTRL_CLK_DIV_RESET
#define	MANTICORE_ROM_MAX_RNG_CLOCK_DIVIDER		HSP_RNG_HW_MAX_CLOCK_DIVIDER
#endif


/**
 * Flash address of internal flash slot A for a 1SP firmware image.
 */
#define	MANTICORE_INTERNAL_SLOT_A_FLASH_ADDRESS		0x000000

/**
 * Flash address of internal flash slot B for a 1SP firmware image.
 */
#define	MANTICORE_INTERNAL_SLOT_B_FLASH_ADDRESS		0x200000

/**
 * Flash address on external flash for a 1SP firmware image.
 */
#define	MANTICORE_EXTERNAL_FLASH_ADDRESS			0x000000


/**
 * Accessor for the HSP sticky register 0.
 */
#define	MANTICORE_HSP_STICKY0_REG	\
	(*((uint32_t*) HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_SW_STICKY_RW_0_ADDRESS))

/**
 * Read the current Boot Order Override value in sticky register 0.
 */
#define	MANTICORE_BOOT_ORDER_OVERRIDE	(MANTICORE_HSP_STICKY0_REG & 0xf)

/**
 * Set the Boot Order Override value in sticky register 0.
 */
#define	MANTICORE_SET_BOOT_ORDER_OVERRIDE(x)	\
	MANTICORE_HSP_STICKY0_REG = ((MANTICORE_HSP_STICKY0_REG & ~0xf) | ((x) & 0xf))

/**
 * Possible values for the ROM boot order.
 */
enum manticore_boot_order {
	MANTICORE_BOOT_ORDER_INTA_INTB_EXT_REC = 0x0,	/**< Internal slot A -> Internal slot B -> External -> Recovery */
	MANTICORE_BOOT_ORDER_INTA_EXT_INTB_REC = 0x1,	/**< Internal slot A -> External -> Internal slot B -> Recovery */
	MANTICORE_BOOT_ORDER_INTA_INTB_REC = 0x2,		/**< Internal slot A -> Internal slot B -> Recovery */
	MANTICORE_BOOT_ORDER_INTA_EXT_REC = 0x3,		/**< Internal slot A -> External -> Recovery */
	MANTICORE_BOOT_ORDER_INTA_REC = 0x4,			/**< Internal slot A -> Recovery */
	MANTICORE_BOOT_ORDER_INTB_INTA_EXT_REC = 0x5,	/**< Internal slot B -> Internal slot A -> External -> Recovery */
	MANTICORE_BOOT_ORDER_INTB_EXT_INTA_REC = 0x6,	/**< Internal slot B -> External -> Internal slot A -> Recovery */
	MANTICORE_BOOT_ORDER_INTB_INTA_REC = 0x7,		/**< Internal slot B -> Internal slot A -> Recovery */
	MANTICORE_BOOT_ORDER_INTB_EXT_REC = 0x8,		/**< Internal slot B -> External -> Recovery */
	MANTICORE_BOOT_ORDER_INTB_REC = 0x9,			/**< Internal slot B -> Recovery */
	MANTICORE_BOOT_ORDER_EXT_INTA_INTB_REC = 0xa,	/**< External -> Internal slot A -> Internal slot B -> Recovery */
	MANTICORE_BOOT_ORDER_EXT_INTB_INTA_REC = 0xb,	/**< External -> Internal slot B -> Internal slot A -> Recovery */
	MANTICORE_BOOT_ORDER_EXT_INTA_REC = 0xc,		/**< External -> Internal slot A -> Recovery */
	MANTICORE_BOOT_ORDER_EXT_INTB_REC = 0xd,		/**< External -> Internal slot B -> Recovery */
	MANTICORE_BOOT_ORDER_EXT_REC = 0xe,				/**< External -> Recovery */
	MANTICORE_BOOT_ORDER_REC = 0xf					/**< Recovery */
};

/**
 * Get the value of the Strap Override bit in sticky register 0.
 */
#define	MANTICORE_STRAP_OVERRIDE		(MANTICORE_HSP_STICKY0_REG & (1U << 4))

/**
 * Set the Strap Override bit in sticky register 0.
 */
#define	MANTICORE_SET_STRAP_OVERRIDE(x)	\
	MANTICORE_HSP_STICKY0_REG = ((MANTICORE_HSP_STICKY0_REG & ~(1U << 4)) | (((x) & 0x1) << 4))


/**
 * Maximum value for the init error count.
 */
#define	MANTICORE_ROM_INIT_ERROR_COUNT_MAX	15

/**
 * Read the current Init Error Count value in sticky register 0.
 */
#define	MANTICORE_ROM_INIT_ERROR_COUNT	((MANTICORE_HSP_STICKY0_REG >> 5) & 0xf)

/**
 * Set the Init Error Count value in sticky register 0.
 */
#define	MANTICORE_ROM_SET_INIT_ERROR_COUNT(x)	\
	MANTICORE_HSP_STICKY0_REG = ((MANTICORE_HSP_STICKY0_REG & ~(0xfU << 5)) | (((x) & 0xf) << 5))

/**
 * Get the value of the Init Complete bit in sticky register 0.
 */
#define	MANTICORE_ROM_INIT_COMPLETE		(MANTICORE_HSP_STICKY0_REG & (1U << 9))

/**
 * Set the Init Complete bit in sticky register 0.
 */
#define	MANTICORE_ROM_SET_INIT_COMPLETE(x)	\
	MANTICORE_HSP_STICKY0_REG = ((MANTICORE_HSP_STICKY0_REG & ~(1U << 9)) | (((x) & 0x1) << 9))

/**
 * Maximum value for the boot error count.
 */
#define	MANTICORE_ROM_BOOT_ERROR_COUNT_MAX	15

/**
 * Read the current Boot Error Count value in sticky register 0.
 */
#define	MANTICORE_ROM_BOOT_ERROR_COUNT	((MANTICORE_HSP_STICKY0_REG >> 10) & 0xf)

/**
 * Set the Boot Error Count value in sticky register 0.
 */
#define	MANTICORE_ROM_SET_BOOT_ERROR_COUNT(x)	\
	MANTICORE_HSP_STICKY0_REG = ((MANTICORE_HSP_STICKY0_REG & ~(0xfU << 10)) | (((x) & 0xf) << 10))


/**
 * Accessor for the HSP scratch register 0.
 */
#define	MANTICORE_HSP_SCRATCH0_REG	\
	(*((uint32_t*) HSP_ADDR_MAP_CREG_MISC_REGS_CREG_MISC_GROUP_HSP_SCRATCH0_ADDRESS))

/**
 * Get the Boot Order value from scratch register 0.
 */
#define	MANTICORE_BOOT_ORDER			(MANTICORE_HSP_SCRATCH0_REG & 0xf)

/**
 * Get the Boot Source value from scratch register 0.
 */
#define	MANTICORE_BOOT_SOURCE			((MANTICORE_HSP_SCRATCH0_REG >> 4) & 0xf)

/**
 * Set the Boot Source value in scratch register 0.
 */
#define	MANTICORE_SET_BOOT_SOURCE(x)	MANTICORE_HSP_SCRATCH0_REG |= (((x) & 0xf) << 4)

/**
 * Indicator for the location where 1SP firmware was loaded from.
 */
enum manticore_boot_source {
	MANTICORE_BOOT_SOURCE_INTA = 0x1,				/**< Internal flash slot A */
	MANTICORE_BOOT_SOURCE_INTB = 0x2,				/**< Internal flash slot B */
	MANTICORE_BOOT_SOURCE_EXT = 0x4,				/**< External flash */
	MANTICORE_BOOT_SOURCE_REC  = 0x8				/**< I2C recovery */
};


/**
 * Accessor for the HSP sticky register 1, which holds the cumulative fatal error log.
 */
#define	MANTICORE_HSP_STICKY1_REG	\
	(*((uint32_t*) HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_SW_STICKY_RW_1_ADDRESS))

/**
 * Accessor for the HSP scratch register 1, which holds the fatal error log for this boot context.
 */
#define	MANTICORE_HSP_SCRATCH1_REG	\
	(*((uint32_t*) HSP_ADDR_MAP_CREG_MISC_REGS_CREG_MISC_GROUP_HSP_SCRATCH1_ADDRESS))

/**
 * Accessor for the HSP sticky register 2, which holds the exception information for the prior ROM
 * execution.
 */
#define	MANTICORE_HSP_STICKY2_REG	\
	(*((uint32_t*) HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_SW_STICKY_RW_2_ADDRESS))

/**
 * Accessor for the HSP sticky register 3, which holds additional exception information for the
 * prior ROM execution.
 */
#define	MANTICORE_HSP_STICKY3_REG	\
	(*((uint32_t*) HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_SW_STICKY_RW_3_ADDRESS))


#endif /* MANTICORE_ROM_H_ */
