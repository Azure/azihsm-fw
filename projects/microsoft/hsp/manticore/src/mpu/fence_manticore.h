
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FENCE_MANTICORE_H_
#define FENCE_MANTICORE_H_

#include "fence_interface.h"
#include "mmio/mmio_register_block.h"


/**
 * Fence blocks register addresses for Manticore
 */
enum {
	FENCE_REGISTER_BLOCK_ADDRESS_BEGIN = 0xB0201000,	/**< Fencing registers blocks start address */
	FENCE_REGISTER_BLOCK_ADDRESS_APB = 0xB0201000,		/**< Registers block address for APB fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_DUAL_CP = 0xB0202000,	/**< Registers block address for CP0/1 cores fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_NQM = 0xB0203000,		/**< Registers block address for FP0/1/2 cores fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_BCP = 0xB0204000,		/**< Registers block address for BCP fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_GDMA = 0xB0205000,		/**< Registers block address for GDMA fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_GSRAM = 0xB0206000,	/**< Registers block address for GSRAM fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_PCIE = 0xB0207000,		/**< Registers block address for PCIE fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_UPKAB0 = 0xB0208000,	/**< Registers block address for UPKAB0 fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_UPKAB1 = 0xB0209000,	/**< Registers block address for UPKAB1 fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_HSSHA = 0xB020A000,	/**< Registers block address for HSSHA fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_AES = 0xB020B000,		/**< Registers block address for AES fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_RNG = 0xB020C000,		/**< Registers block address for RNG fencing block */
	FENCE_REGISTER_BLOCK_ADDRESS_END = 0xB020D000,		/**< Fencing registers blocks end address */
};

/**
 * Memory fence description block
 */
struct memory_fencing_block_info {
	uint32_t max_block_entries_number;	/**< Maximum number of entries per fence block*/
	uint32_t register_block_address;	/**< Fence register block address */
};

/**
 * Fence entry registers structure
 */
struct fence_entry_manticore {
	struct {
		uint32_t region_end_address;		/**< Memory region end address */
		uint32_t region_start_address;		/**< Memory region start address */
		uint32_t read_access_low_bits;		/**< Read access low 32 bits */
		uint32_t read_access_high_bits;		/**< Read access high 32 bits */
		uint32_t write_access_low_bits;		/**< Write access low 32 bits */
		uint32_t write_access_high_bits;	/**< Write access high 32 bits */
	} registers;							/**< Working set of fence entry registers */
	uint32_t reserved0;						/**< Reserved to comply with register block layout */
	uint32_t reserved1;						/**< Reserved to comply with register block layout */
};

/**
 * Memory fencing implementation for Manticore
 */
struct fence_manticore {
	struct fence_interface base;						/**< Memory fence interface */
	const struct mmio_register_block *fence_registers;	/**< Memory fence registers block */
};


int fence_manticore_init (struct fence_manticore *fence,
	const struct mmio_register_block *fence_registers);
void fence_manticore_release (const struct fence_manticore *fence);


#endif	// FENCE_MANTICORE_H_
