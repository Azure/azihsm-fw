// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TDISP_TDI_CONTEXT_MANAGER_MANTICORE_STATIC_H_
#define TDISP_TDI_CONTEXT_MANAGER_MANTICORE_STATIC_H_

#include "tdisp_tdi_context_manager_manticore.h"


/**
 * Interface functions prototypes
 */
int tdisp_tdi_context_manager_manticore_clear_tdi_context (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_id);
int tdisp_tdi_context_manager_manticore_clear_all_tdi_context (
	const struct tdisp_tdi_context_manager *mgr);
int tdisp_tdi_context_manager_manticore_get_tdi_context (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_id, uint32_t context_mask,
	struct tdisp_tdi_context *context);
int tdisp_tdi_context_manager_manticore_set_start_nonce (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_id, const uint8_t *nonce,
	size_t nonce_size);
int tdisp_tdi_context_manager_manticore_set_lock_flags (const struct tdisp_tdi_context_manager *mgr,
	uint32_t tdi_id, uint16_t lock_flags);
int tdisp_tdi_context_manager_manticore_set_default_ide_stream (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_id, uint8_t ide_stream_id);
int tdisp_tdi_context_manager_manticore_set_mmio_reporting_offset (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_id, uint64_t mmio_reporting_offset);
int tdisp_tdi_context_manager_manticore_set_bind_p2p_address_mask (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_id, uint64_t bind_p2p_address_mask);
int tdisp_tdi_context_manager_manticore_set_reserved (const struct tdisp_tdi_context_manager *mgr,
	uint32_t tdi_id, uint8_t index, uint32_t value);


/**
 * Manticore TDISP TDI context manager interface API initialization
 */
#define TDISP_TDI_CONTEXT_MANAGER_MANTICORE_API_INIT { \
	.clear_tdi_context = tdisp_tdi_context_manager_manticore_clear_tdi_context, \
	.clear_all_tdi_context = tdisp_tdi_context_manager_manticore_clear_all_tdi_context, \
	.get_tdi_context = tdisp_tdi_context_manager_manticore_get_tdi_context, \
	.set_start_nonce = tdisp_tdi_context_manager_manticore_set_start_nonce, \
	.set_lock_flags = tdisp_tdi_context_manager_manticore_set_lock_flags, \
	.set_mmio_reporting_offset = tdisp_tdi_context_manager_manticore_set_mmio_reporting_offset, \
	.set_default_ide_stream = tdisp_tdi_context_manager_manticore_set_default_ide_stream, \
	.set_bind_p2p_address_mask = tdisp_tdi_context_manager_manticore_set_bind_p2p_address_mask, \
	.set_reserved = tdisp_tdi_context_manager_manticore_set_reserved, \
}

/**
 * Static initializer for Manticore TDISP TDI context manager implementation
 *
 * @param tdi_context_soc_address_arg SOC address for TDI context entries array
 * @param tdi_context_entries_count_arg TDI context entries count
 * @param dmb_ptr Pointer to DMB instance
 */
#define tdisp_tdi_context_manager_manticore_static_init(tdi_context_soc_address_arg, \
	tdi_context_entries_count_arg, dmb_ptr) { \
	.base = TDISP_TDI_CONTEXT_MANAGER_MANTICORE_API_INIT, \
	.tdi_context_soc_address = tdi_context_soc_address_arg, \
	.tdi_context_entries_count = tdi_context_entries_count_arg, \
	.dmb = dmb_ptr, \
}


#endif	// TDISP_TDI_CONTEXT_MANAGER_MANTICORE_STATIC_H_
