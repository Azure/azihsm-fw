// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FENCE_H_
#define FENCE_H_

#include <stdbool.h>
#include <stdint.h>
#include "common/sram_util.h"
#include "status/manticore_module_id.h"


/**
 * Memory fencing logical blocks IDs. Used as an index in description array
 */
enum {
	FENCE_BLOCK_APB,
	FENCE_BLOCK_DUAL_CP,
	FENCE_BLOCK_NQM,
	FENCE_BLOCK_BCP,
	FENCE_BLOCK_GDMA,
	FENCE_BLOCK_GSRAM,
	FENCE_BLOCK_PCIE,
	FENCE_BLOCK_UPKAB0,
	FENCE_BLOCK_UPKAB1,
	FENCE_BLOCK_HSSHA,
	FENCE_BLOCK_AES,
	FENCE_BLOCK_RNG,
	FENCE_BLOCK_COUNT,
};

/**
 * AXI bus initiator bits corresponding to initiators IDs for programming SOC level memory fencing
 */
enum {
	FENCE_INITIATOR_HSP = 0,
	FENCE_INITIATOR_CP0 = 1,
	FENCE_INITIATOR_CP1 = 2,
	FENCE_INITIATOR_HSSHA = 3,
	FENCE_INITIATOR_AES = 4,
	FENCE_INITIATOR_PCIE = 5,
	FENCE_INITIATOR_QMGR_FP = 6,
	FENCE_INITIATOR_QMGR_CMD_FETCHER = 7,
	FENCE_INITIATOR_QMGR_MSIX = 8,
	FENCE_INITIATOR_SPIS0 = 9,
	FENCE_INITIATOR_BCP_CDMA = 10,
	FENCE_INITIATOR_GDMA = 11,
	FENCE_INITIATOR_CS_DAP_DBG = 12,
	FENCE_INITIATOR_CS_ETR = 13,
	FENCE_INITIATOR_SPIS1 = 14,
	FENCE_INITIATOR_UPKA0 = 31,
	FENCE_INITIATOR_UPKA1 = 32,
	FENCE_INITIATOR_UPKA2 = 33,
	FENCE_INITIATOR_UPKA3 = 34,
	FENCE_INITIATOR_UPKA4 = 35,
	FENCE_INITIATOR_UPKA5 = 36,
	FENCE_INITIATOR_UPKA6 = 37,
	FENCE_INITIATOR_UPKA7 = 38,
	FENCE_INITIATOR_UPKA8 = 39,
	FENCE_INITIATOR_UPKA9 = 40,
	FENCE_INITIATOR_UPKA10 = 41,
	FENCE_INITIATOR_UPKA11 = 42,
	FENCE_INITIATOR_UPKA12 = 43,
	FENCE_INITIATOR_UPKA13 = 44,
	FENCE_INITIATOR_UPKA14 = 45,
	FENCE_INITIATOR_UPKA15 = 46,
};

/**
 * Memory fencing initiators masks
 */
enum {
	FENCE_INITIATOR_MASK_HSP = (1LLU << FENCE_INITIATOR_HSP),
	FENCE_INITIATOR_MASK_CP0 = (1LLU << FENCE_INITIATOR_CP0),
	FENCE_INITIATOR_MASK_CP1 = (1LLU << FENCE_INITIATOR_CP1),
	FENCE_INITIATOR_MASK_HSSHA = (1LLU << FENCE_INITIATOR_HSSHA),
	FENCE_INITIATOR_MASK_AES = (1LLU << FENCE_INITIATOR_AES),
	FENCE_INITIATOR_MASK_PCIE = (1LLU << FENCE_INITIATOR_PCIE),
	FENCE_INITIATOR_MASK_QMGR_FP = (1LLU << FENCE_INITIATOR_QMGR_FP),
	FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER = (1LLU << FENCE_INITIATOR_QMGR_CMD_FETCHER),
	FENCE_INITIATOR_MASK_QMGR_MSIX = (1LLU << FENCE_INITIATOR_QMGR_MSIX),
	FENCE_INITIATOR_MASK_SPIS0 = (1LLU << FENCE_INITIATOR_SPIS0),
	FENCE_INITIATOR_MASK_BCP_CDMA = (1LLU << FENCE_INITIATOR_BCP_CDMA),
	FENCE_INITIATOR_MASK_GDMA = (1LLU << FENCE_INITIATOR_GDMA),
	FENCE_INITIATOR_MASK_CS_DAP_DBG = (1LLU << FENCE_INITIATOR_CS_DAP_DBG),
	FENCE_INITIATOR_MASK_CS_ETR = (1LLU << FENCE_INITIATOR_CS_ETR),
	FENCE_INITIATOR_MASK_SPIS1 = (1LLU << FENCE_INITIATOR_SPIS1),
	FENCE_INITIATOR_MASK_UPKA0 = (1LLU << FENCE_INITIATOR_UPKA0),
	FENCE_INITIATOR_MASK_UPKA1 = (1LLU << FENCE_INITIATOR_UPKA1),
	FENCE_INITIATOR_MASK_UPKA2 = (1LLU << FENCE_INITIATOR_UPKA2),
	FENCE_INITIATOR_MASK_UPKA3 = (1LLU << FENCE_INITIATOR_UPKA3),
	FENCE_INITIATOR_MASK_UPKA4 = (1LLU << FENCE_INITIATOR_UPKA4),
	FENCE_INITIATOR_MASK_UPKA5 = (1LLU << FENCE_INITIATOR_UPKA5),
	FENCE_INITIATOR_MASK_UPKA6 = (1LLU << FENCE_INITIATOR_UPKA6),
	FENCE_INITIATOR_MASK_UPKA7 = (1LLU << FENCE_INITIATOR_UPKA7),
	FENCE_INITIATOR_MASK_UPKA8 = (1LLU << FENCE_INITIATOR_UPKA8),
	FENCE_INITIATOR_MASK_UPKA9 = (1LLU << FENCE_INITIATOR_UPKA9),
	FENCE_INITIATOR_MASK_UPKA10 = (1LLU << FENCE_INITIATOR_UPKA10),
	FENCE_INITIATOR_MASK_UPKA11 = (1LLU << FENCE_INITIATOR_UPKA11),
	FENCE_INITIATOR_MASK_UPKA12 = (1LLU << FENCE_INITIATOR_UPKA12),
	FENCE_INITIATOR_MASK_UPKA13 = (1LLU << FENCE_INITIATOR_UPKA13),
	FENCE_INITIATOR_MASK_UPKA14 = (1LLU << FENCE_INITIATOR_UPKA14),
	FENCE_INITIATOR_MASK_UPKA15 = (1LLU << FENCE_INITIATOR_UPKA15),
	FENCE_INITIATOR_MASK_ALL = (FENCE_INITIATOR_MASK_HSP |
		FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_HSSHA |
		FENCE_INITIATOR_MASK_AES | FENCE_INITIATOR_MASK_PCIE | FENCE_INITIATOR_MASK_QMGR_FP |
		FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER | FENCE_INITIATOR_MASK_QMGR_MSIX |
		FENCE_INITIATOR_MASK_SPIS0 | FENCE_INITIATOR_MASK_BCP_CDMA | FENCE_INITIATOR_MASK_GDMA |
		FENCE_INITIATOR_MASK_CS_DAP_DBG | FENCE_INITIATOR_MASK_CS_ETR |
		FENCE_INITIATOR_MASK_SPIS1 | FENCE_INITIATOR_MASK_UPKA0 | FENCE_INITIATOR_MASK_UPKA1 |
		FENCE_INITIATOR_MASK_UPKA2 | FENCE_INITIATOR_MASK_UPKA3 | FENCE_INITIATOR_MASK_UPKA4 |
		FENCE_INITIATOR_MASK_UPKA5 | FENCE_INITIATOR_MASK_UPKA6 | FENCE_INITIATOR_MASK_UPKA7 |
		FENCE_INITIATOR_MASK_UPKA8 | FENCE_INITIATOR_MASK_UPKA9 | FENCE_INITIATOR_MASK_UPKA10 |
		FENCE_INITIATOR_MASK_UPKA11 | FENCE_INITIATOR_MASK_UPKA12 | FENCE_INITIATOR_MASK_UPKA13 |
		FENCE_INITIATOR_MASK_UPKA14 | FENCE_INITIATOR_MASK_UPKA15),
};

/** Fencing logic entry which defines a single memory region access. Application layer has to
 * define up to 64 bits masks which correspond to specific device initiator IDs
 */
struct fence_policy_entry {
	struct sram_block memory_region;	/**< Memory region to be configured */
	uint64_t read_access_bits;			/**< Up to 64 bits of initiators for read access */
	uint64_t write_access_bits;			/**< Up to 64 bits of initiators for write access */
};

/**
 * Fence logic policy block struct, which contains an array of policy entries
 */
struct fence_policy_block {
	uint32_t fence_block_id;						/**< Fence logic block ID */
	const struct fence_policy_entry *block_entries;	/**< Array of fence policy entries */
	uint32_t block_entries_count;					/**< Count of policy etnries */
};

/**
 * Memory fencing interface
 */
struct fence_interface {
	/**
	 * Applies fence policy which consist of set of fence logic policy
	 * blocks. The entire previous policy is fully reset before aplying new one.
	 *
	 * @param fence - fence interface instance
	 * @param fence_blocks - array of fence logic blocks, must be not NULL unless blocks_count is 0
	 * @param blocks_count - number of fence logic blocks. Allowed to be 0 to reset previous policy
	 *
	 * @return 0 if successful, error code otherwise
	 */
	int (*apply) (const struct fence_interface *fence,
		const struct fence_policy_block *fence_blocks, uint32_t blocks_count);
};

#define FENCE_ERROR(code)    ROT_ERROR (MANTICORE_MODULE_FENCE, code)

/**
 * Error codes that can be generated by fence interface.
 */
enum {
	FENCE_INVALID_ARGUMENT = FENCE_ERROR (0x00),			/**< Input parameter is null or not valid. */
	FENCE_NO_MEMORY = FENCE_ERROR (0x01),					/**< Memory allocation failed. */
	FENCE_APPLY_FAILED = FENCE_ERROR (0x02),				/**< Fence apply failure */
	FENCE_INVALID_FENCE_BLOCK_ID = FENCE_ERROR (0x03),		/**< Invalid fence block ID */
	FENCE_TOO_MANY_ENTRIES_FOR_BLOCK = FENCE_ERROR (0x04),	/**< Too many entries specified for a block */
	FENCE_INVALID_MEMORY_REGION = FENCE_ERROR (0x05),		/**< Invalid memory region, 0 size */
	FENCE_REGION_ADDRESS_MISALIGNED = FENCE_ERROR (0x06),	/**< Memory region address is not 4k aligned */
	FENCE_REGION_SIZE_MISALIGNED = FENCE_ERROR (0x07),		/**< Memory region size is not 4k aligned */
	FENCE_INVALID_ACCESS_MASK = FENCE_ERROR (0x08),			/**< Invalid access mask */
};


#endif	// FENCE_H_
