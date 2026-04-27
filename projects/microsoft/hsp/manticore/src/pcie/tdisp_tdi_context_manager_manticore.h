// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TDISP_TDI_CONTEXT_MANAGER_MANTICORE_H_
#define TDISP_TDI_CONTEXT_MANAGER_MANTICORE_H_

#include "drivers/hsp_dmb.h"
#include "pcisig/tdisp/tdisp_tdi_context_manager.h"


/**
 * Maximum supported number of TDI interfaces (64 VFs + 1 PF)
 */
#define TDISP_TDI_CONTEXT_MAX_COUNT 65

/**
 * Manticore implementaton of TDISP TDI context manager
 */
struct tdisp_tdi_context_manager_manticore {
	struct tdisp_tdi_context_manager base;	/**< TDI context manager interface */
	uint64_t tdi_context_soc_address;		/**< SOC address for TDI context registers */
	size_t tdi_context_entries_count;		/**< Size of TDI context array */
	const struct hsp_dmb *dmb;				/**< DMB interface for accessing TDI context */
};


int tdisp_tdi_context_manager_manticore_init (struct tdisp_tdi_context_manager_manticore *mgr,
	uint64_t tdi_context_soc_address, size_t tdi_context_entries_count, const struct hsp_dmb *dmb);
void tdisp_tdi_context_manager_manticore_release (
	const struct tdisp_tdi_context_manager_manticore *mgr);


#endif	// TDISP_TDI_CONTEXT_MANAGER_MANTICORE_H_
