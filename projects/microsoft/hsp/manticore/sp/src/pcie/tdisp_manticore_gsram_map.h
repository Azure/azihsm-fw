#ifndef TDISP_MANTICORE_GSRAM_MAP_H_
#define TDISP_MANTICORE_GSRAM_MAP_H_

#include "rot_memory_map.h"
#include "crypto/hash_hs_sha_soc.h"
#include "pcie/ide_driver_manticore.h"
#include "pcie/tdisp_tdi_context_manager_manticore.h"
#include "spdm/spdm_persistent_context_manticore_gsram.h"

/**
 * Total size of the GSRAM region allocated for TDISP and other components
 */
#define TDISP_MANTICORE_CONTEXT_GSRAM_SIZE GSRAM_MEM_MAP_IDE_KM_CONTEXT_SIZE

/**
 * This struct is used to define TDISP context layout map. Offsets of its members will be used
 * to define absolute GSRAM addresses for various components.
 * NOTE: This struct (or any its dependencies) CAN NOT be changed , otherwise it will trigger
 * the need for IMPACTFUL update, which is not desirable.
 */
struct tdisp_manticore_gsram_map {
	struct ide_driver_manticore_key_context ide_context;							/**< IDE driver context */
	struct spdm_persistent_context_manticore_gsram_data spdm_persistent_context;	/**< SPDM persistent context */
	struct hs_sha_multi_update spdm_transcript_hash_state[4];						/**< SHA hardware hash engine */
	struct tdisp_tdi_context tdisp_tdi_contexts[TDISP_TDI_CONTEXT_MAX_COUNT];		/**< TDI context manager */
	uint32_t end_of_context;														/**< End of context marker to verify size of structure */
};


/**
 * NOTE: It is important to keep these structs as is to make sure binary compatibility
 * with previous versions
 */
_Static_assert (offsetof (struct tdisp_manticore_gsram_map, ide_context) == 0,
	"Unexpected struct member offset");
_Static_assert (offsetof (struct tdisp_manticore_gsram_map, spdm_persistent_context) == 1056,
	"Unexpected struct member offset");
_Static_assert (offsetof (struct tdisp_manticore_gsram_map, spdm_transcript_hash_state) == 1856,
	"Unexpected struct member offset");
_Static_assert (offsetof (struct tdisp_manticore_gsram_map, tdisp_tdi_contexts) == 2688,
	"Unexpected struct member offset");

/**
 * This offset represent the size of used space inside TDISP context */
_Static_assert (offsetof (struct tdisp_manticore_gsram_map, end_of_context) == 7368,
	"Unexpected struct member offset");

/**
 * Check to make sure TDISP context fits into GSRAM allocated space */
_Static_assert (offsetof (struct tdisp_manticore_gsram_map,
	end_of_context) <= TDISP_MANTICORE_CONTEXT_GSRAM_SIZE, "TDISP context size is overflwn");

/**
 * SoC address of IDE key context check for IDFU breaking changes
 */
_Static_assert (IDE_KEY_CONTEXT_ADDRESS == 0x61003000, "IDE key context IDFU breaking change");

/**
 * SoC address of SPDM persistent context
 */
#define SPDM_PERSISTENT_CONTEXT_GSRAM_ADDRESS \
		(IDE_KEY_CONTEXT_ADDRESS + offsetof(struct tdisp_manticore_gsram_map, spdm_persistent_context))
_Static_assert (SPDM_PERSISTENT_CONTEXT_GSRAM_ADDRESS == 0x61003420,
	"SPDM persistent context IDFU breaking change");

/**
 * SoC addresses of SPDM transcript hash engine states 0-3
 * Note: 4 instances are required to support simultaneous SPDM transcripts
 */
#define SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS0 \
		(IDE_KEY_CONTEXT_ADDRESS + offsetof(struct tdisp_manticore_gsram_map, spdm_transcript_hash_state[0]))
_Static_assert ((SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS0 == 0x61003740),
	"SPDM transcript hash state 0 IDFU breaking change");
#define SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS1 \
		(IDE_KEY_CONTEXT_ADDRESS + offsetof(struct tdisp_manticore_gsram_map, spdm_transcript_hash_state[1]))
_Static_assert (SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS1 == 0x61003810,
	"SPDM transcript hash state 1 IDFU breaking change");
#define SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS2 \
		(IDE_KEY_CONTEXT_ADDRESS + offsetof(struct tdisp_manticore_gsram_map, spdm_transcript_hash_state[2]))
_Static_assert (SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS2 == 0x610038e0,
	"SPDM transcript hash state 2 IDFU breaking change");
#define SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS3 \
		(IDE_KEY_CONTEXT_ADDRESS + offsetof(struct tdisp_manticore_gsram_map, spdm_transcript_hash_state[3]))
_Static_assert (SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS3 == 0x610039b0,
	"SPDM transcript hash state 3 IDFU breaking change");

/**
 * SoC address of TDISP TDI contexts
 */
#define TDISP_TDI_CONTEXTS_GSRAM_ADDRESS \
		(IDE_KEY_CONTEXT_ADDRESS + offsetof(struct tdisp_manticore_gsram_map, tdisp_tdi_contexts))
_Static_assert (TDISP_TDI_CONTEXTS_GSRAM_ADDRESS == 0x61003a80,
	"TDISP TDI contexts IDFU breaking change");

/**
 * SoC address of end of context marker to verify size of structure
 */
#define TDISP_MANTICORE_END_OF_CONTEXT_GSRAM_ADDRESS \
		(IDE_KEY_CONTEXT_ADDRESS + offsetof(struct tdisp_manticore_gsram_map, end_of_context))
_Static_assert (TDISP_MANTICORE_END_OF_CONTEXT_GSRAM_ADDRESS == 0x61004cc8,
	"End of context IDFU breaking change");


#endif	// TDISP_MANTICORE_GSRAM_MAP_H_
