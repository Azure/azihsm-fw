// Copyright (c) Microsoft Corporation. All rights reserved.

#include "fence_manticore.h"
#include "common/array_size.h"
#include "common/type_cast.h"
#include "common/unused.h"


/**
 * Fence blocks description array used for local implementation
 */
const struct memory_fencing_block_info manticore_fencing_info[FENCE_BLOCK_COUNT] = {
	[FENCE_BLOCK_APB] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_APB,
	},
	[FENCE_BLOCK_DUAL_CP] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_DUAL_CP,
	},
	[FENCE_BLOCK_NQM] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_NQM,
	},
	[FENCE_BLOCK_BCP] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_BCP,
	},
	[FENCE_BLOCK_GDMA] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_GDMA,
	},
	[FENCE_BLOCK_GSRAM] = {
		.max_block_entries_number = 10,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_GSRAM,
	},
	[FENCE_BLOCK_PCIE] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_PCIE,
	},
	[FENCE_BLOCK_UPKAB0] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_UPKAB0,
	},
	[FENCE_BLOCK_UPKAB1] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_UPKAB1,
	},
	[FENCE_BLOCK_HSSHA] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_HSSHA,
	},
	[FENCE_BLOCK_AES] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_AES,
	},
	[FENCE_BLOCK_RNG] = {
		.max_block_entries_number = 8,
		.register_block_address = FENCE_REGISTER_BLOCK_ADDRESS_RNG,
	},
};


/**
 * Helper function which clears any memory fencing for all available fence blocks
 *
 * @param fencing - memory fencing interface instance
 *
 * @return 0 if successful, error code otherwise
 */
static int fence_manticore_clear (const struct fence_manticore *fencing)
{
	uint32_t i, j;
	const struct memory_fencing_block_info *block_info;
	int status;
	uint32_t register_block_offset;
	const struct fence_entry_manticore default_fence_entry = {
		.registers = {
			.region_end_address = 0x00000FFF,
			.region_start_address = 0x00000000,
			.read_access_low_bits = 0x1,
			.read_access_high_bits = 0x0,
			.write_access_low_bits = 0x1,
			.write_access_high_bits = 0x0,
		},
		.reserved0 = 0,
		.reserved1 = 0,
	};

	status = fencing->fence_registers->map (fencing->fence_registers);
	if (status != 0) {
		return status;
	}

	/* For each fencing block zero out all entries */
	for (i = 0; i < ARRAY_SIZE (manticore_fencing_info); i++) {
		block_info = &manticore_fencing_info[i];
		register_block_offset = block_info->register_block_address -
			FENCE_REGISTER_BLOCK_ADDRESS_BEGIN;

		for (j = 0; j < block_info->max_block_entries_number; j++) {
			status = fencing->fence_registers->block_write32 (fencing->fence_registers,
				register_block_offset + (j * sizeof (default_fence_entry)),
				&default_fence_entry.registers.region_end_address,
				sizeof (default_fence_entry.registers) / sizeof (uint32_t));
			if (status != 0) {
				goto exit;
			}
		}
	}

exit:
	fencing->fence_registers->unmap (fencing->fence_registers);

	return status;
}

/**
 * Helper function to apply memory fencing entries for specified fence block. This function
 * doesn't validate input arguments, assuming that the caller has done all required validations
 *
 * @param fencing - memory fencing interface instance
 * @param block_info - fence block info
 * @param block_entries - fence entries to be applied for given block
 * @param block_entries_count - number of entries to be applied
 *
 * @return 0 if successful, error code otherwise
 */
static int fence_manticore_apply_block_policy (const struct fence_manticore *fencing,
	const struct memory_fencing_block_info *block_info,
	const struct fence_policy_entry *block_entries,	uint32_t block_entries_count)
{
	uint32_t i;
	int status;
	struct fence_entry_manticore entry;
	uint32_t register_block_offset = block_info->register_block_address -
		FENCE_REGISTER_BLOCK_ADDRESS_BEGIN;

	status = fencing->fence_registers->map (fencing->fence_registers);
	if (status != 0) {
		return status;
	}

	for (i = 0; i < block_entries_count; i++) {
		entry.registers.region_start_address = (uintptr_t) block_entries[i].memory_region.start;
		entry.registers.region_end_address = entry.registers.region_start_address +
			block_entries[i].memory_region.length - 1;
		entry.registers.read_access_low_bits = (block_entries[i].read_access_bits & 0xffffffff) |
			FENCE_INITIATOR_MASK_HSP;
		entry.registers.read_access_high_bits = block_entries[i].read_access_bits >> 32;
		entry.registers.write_access_low_bits = (block_entries[i].write_access_bits & 0xffffffff) |
			FENCE_INITIATOR_MASK_HSP;
		entry.registers.write_access_high_bits = block_entries[i].write_access_bits >> 32;

		status = fencing->fence_registers->block_write32 (fencing->fence_registers,
			register_block_offset + (i * sizeof (entry)), &entry.registers.region_end_address,
			sizeof (entry.registers) / sizeof (uint32_t));
		if (status != 0) {
			goto exit;
		}
	}

exit:
	fencing->fence_registers->unmap (fencing->fence_registers);

	return status;
}

int fence_manticore_apply (const struct fence_interface *fence,
	const struct fence_policy_block *fence_blocks, uint32_t blocks_count)
{
	uint32_t i, j;
	const struct memory_fencing_block_info *block_info;
	int status;
	const struct fence_policy_block *block;
	const struct fence_policy_entry *entry;
	const struct fence_manticore *fencing = TO_DERIVED_TYPE (fence, const struct fence_manticore,
		base);

	if (fence == NULL) {
		return FENCE_INVALID_ARGUMENT;
	}

	if (((fence_blocks == NULL) && (blocks_count > 0)) ||
		((fence_blocks != NULL) && (blocks_count == 0))) {
		return FENCE_INVALID_ARGUMENT;
	}

	/* Run validation of the entire policy before applying any entries */
	for (i = 0; i < blocks_count; i++) {
		block = &fence_blocks[i];

		if (block->fence_block_id >= FENCE_BLOCK_COUNT) {
			return FENCE_INVALID_FENCE_BLOCK_ID;
		}

		block_info = &manticore_fencing_info[block->fence_block_id];

		if (block->block_entries_count > block_info->max_block_entries_number) {
			return FENCE_TOO_MANY_ENTRIES_FOR_BLOCK;
		}

		for (j = 0; j < block->block_entries_count; j++) {
			entry = &block->block_entries[j];
			if (entry->memory_region.length == 0) {
				return FENCE_INVALID_MEMORY_REGION;
			}

			if ((entry->memory_region.length & 0xFFF) != 0) {
				return FENCE_REGION_SIZE_MISALIGNED;
			}

			if ((((uintptr_t) entry->memory_region.start) & 0xFFF) != 0) {
				return FENCE_REGION_ADDRESS_MISALIGNED;
			}

			if (((entry->read_access_bits & ~FENCE_INITIATOR_MASK_ALL) != 0) ||
				((entry->write_access_bits & ~FENCE_INITIATOR_MASK_ALL) != 0)) {
				return FENCE_INVALID_ACCESS_MASK;
			}
		}
	}

	status = fence_manticore_clear (fencing);
	if (status != 0) {
		return status;
	}

	/* The entire policy has been validated, just apply it */
	for (i = 0; i < blocks_count; i++) {
		block = &fence_blocks[i];
		block_info = &manticore_fencing_info[block->fence_block_id];

		status = fence_manticore_apply_block_policy (fencing, block_info, block->block_entries,
			block->block_entries_count);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}

/**
 * Initialize memory fencing for Manticore
 *
 * @param fencing memory fencing object instance
 * @param fence_registers - memory fencing registers
 *
 * @return 0 if successful, error code otherwise
 */
int fence_manticore_init (struct fence_manticore *fencing,
	const struct mmio_register_block *fence_registers)
{
	if ((fencing == NULL) || (fence_registers == NULL)) {
		return FENCE_INVALID_ARGUMENT;
	}

	fencing->base.apply = fence_manticore_apply;
	fencing->fence_registers = fence_registers;

	return 0;
}

/**
 * Release memory fencing object resources
 *
 * @param fencing memory fencing object instance to clear
 */
void fence_manticore_release (const struct fence_manticore *fencing)
{
	UNUSED (fencing);
}
