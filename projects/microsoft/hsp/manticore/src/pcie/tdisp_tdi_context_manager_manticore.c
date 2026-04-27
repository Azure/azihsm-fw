// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "tdisp_tdi_context_manager_manticore.h"
#include "common/array_size.h"
#include "common/type_cast.h"
#include "common/unused.h"

int tdisp_tdi_context_manager_manticore_clear_tdi_context (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_index)
{
	const struct tdisp_tdi_context_manager_manticore *mmgr = TO_DERIVED_TYPE (mgr,
		const struct tdisp_tdi_context_manager_manticore, base);
	struct tdisp_tdi_context *tdi_contexts = NULL;
	int status;

	if (mgr == NULL) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	if (tdi_index >= mmgr->tdi_context_entries_count) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_INTERFACE;
	}

	status = mmgr->dmb->map_soc_address (mmgr->dmb, mmgr->tdi_context_soc_address,
		sizeof (struct tdisp_tdi_context) * mmgr->tdi_context_entries_count, HSP_DMB_ACCESS_WRITE,
		(void**) &tdi_contexts);

	if (status != 0) {
		return status;
	}

	status = tdisp_tdi_context_clear (&tdi_contexts[tdi_index]);

	mmgr->dmb->unmap_soc_address (mmgr->dmb, tdi_contexts);

	return status;
}

int tdisp_tdi_context_manager_manticore_clear_all_tdi_context (
	const struct tdisp_tdi_context_manager *mgr)
{
	const struct tdisp_tdi_context_manager_manticore *mmgr = TO_DERIVED_TYPE (mgr,
		const struct tdisp_tdi_context_manager_manticore, base);
	struct tdisp_tdi_context *tdi_contexts = NULL;
	int status;

	if (mgr == NULL) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	status = mmgr->dmb->map_soc_address (mmgr->dmb, mmgr->tdi_context_soc_address,
		sizeof (struct tdisp_tdi_context) * mmgr->tdi_context_entries_count, HSP_DMB_ACCESS_WRITE,
		(void**) &tdi_contexts);

	if (status != 0) {
		return status;
	}

	memset (tdi_contexts, 0, sizeof (tdi_contexts[0]) * mmgr->tdi_context_entries_count);

	mmgr->dmb->unmap_soc_address (mmgr->dmb, tdi_contexts);

	return 0;
}

int tdisp_tdi_context_manager_manticore_get_tdi_context (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_index, uint32_t context_mask,
	struct tdisp_tdi_context *context)
{
	const struct tdisp_tdi_context_manager_manticore *mmgr = TO_DERIVED_TYPE (mgr,
		const struct tdisp_tdi_context_manager_manticore, base);
	struct tdisp_tdi_context *tdi_contexts = NULL;
	int status;

	if ((mgr == NULL) || (context == NULL)) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	if (tdi_index >= mmgr->tdi_context_entries_count) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_INTERFACE;
	}

	if ((context_mask & ~TDISP_TDI_CONTEXT_MASK_ALL) != 0) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_MASK;
	}

	status = mmgr->dmb->map_soc_address (mmgr->dmb, mmgr->tdi_context_soc_address,
		sizeof (struct tdisp_tdi_context) * mmgr->tdi_context_entries_count, HSP_DMB_ACCESS_READ,
		(void**) &tdi_contexts);

	if (status != 0) {
		return status;
	}

	context->tdi_context_mask = tdi_contexts[tdi_index].tdi_context_mask;

	if (((context_mask & TDISP_TDI_CONTEXT_MASK_NONCE) != 0) &&
		((context->tdi_context_mask & TDISP_TDI_CONTEXT_MASK_NONCE) != 0)) {
		memcpy (context->start_interface_nonce,	tdi_contexts[tdi_index].start_interface_nonce,
			sizeof (tdi_contexts[tdi_index].start_interface_nonce));
	}
	if (((context_mask & TDISP_TDI_CONTEXT_MASK_LOCK_FLAGS) != 0) &&
		((context->tdi_context_mask & TDISP_TDI_CONTEXT_MASK_LOCK_FLAGS) != 0)) {
		context->lock_flags = tdi_contexts[tdi_index].lock_flags;
	}
	if (((context_mask & TDISP_TDI_CONTEXT_MASK_MMIO_REPORTING_OFFSET) != 0) &&
		((context->tdi_context_mask & TDISP_TDI_CONTEXT_MASK_MMIO_REPORTING_OFFSET) != 0)) {
		context->mmio_reporting_offset = tdi_contexts[tdi_index].mmio_reporting_offset;
	}
	if (((context_mask & TDISP_TDI_CONTEXT_MASK_DEFAULT_IDE_STREAM_ID) != 0) &&
		((context->tdi_context_mask & TDISP_TDI_CONTEXT_MASK_DEFAULT_IDE_STREAM_ID) != 0)) {
		context->default_ide_stream_id = tdi_contexts[tdi_index].default_ide_stream_id;
	}
	if (((context_mask & TDISP_TDI_CONTEXT_MASK_BIND_P2P_ADDRESS_MASK) != 0) &&
		((context->tdi_context_mask & TDISP_TDI_CONTEXT_MASK_BIND_P2P_ADDRESS_MASK) != 0)) {
		context->bind_p2p_address_mask = tdi_contexts[tdi_index].bind_p2p_address_mask;
	}
	if (((context_mask & TDISP_TDI_CONTEXT_MASK_RESERVED_0) != 0) &&
		((context->tdi_context_mask & TDISP_TDI_CONTEXT_MASK_RESERVED_0) != 0)) {
		context->reserved[0] = tdi_contexts[tdi_index].reserved[0];
	}
	if (((context_mask & TDISP_TDI_CONTEXT_MASK_RESERVED_1) != 0) &&
		((context->tdi_context_mask & TDISP_TDI_CONTEXT_MASK_RESERVED_1) != 0)) {
		context->reserved[1] = tdi_contexts[tdi_index].reserved[1];
	}
	if (((context_mask & TDISP_TDI_CONTEXT_MASK_RESERVED_2) != 0) &&
		((context->tdi_context_mask & TDISP_TDI_CONTEXT_MASK_RESERVED_2) != 0)) {
		context->reserved[2] = tdi_contexts[tdi_index].reserved[2];
	}
	if (((context_mask & TDISP_TDI_CONTEXT_MASK_RESERVED_3) != 0) &&
		((context->tdi_context_mask & TDISP_TDI_CONTEXT_MASK_RESERVED_3) != 0)) {
		context->reserved[3] = tdi_contexts[tdi_index].reserved[3];
	}

	mmgr->dmb->unmap_soc_address (mmgr->dmb, tdi_contexts);

	return 0;
}

int tdisp_tdi_context_manager_manticore_set_start_nonce (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_index, const uint8_t *nonce,
	size_t nonce_size)
{
	const struct tdisp_tdi_context_manager_manticore *mmgr = TO_DERIVED_TYPE (mgr,
		const struct tdisp_tdi_context_manager_manticore, base);
	struct tdisp_tdi_context *tdi_contexts = NULL;
	int status;

	if ((mgr == NULL) || (nonce == NULL) || (nonce_size != TDISP_START_INTERFACE_NONCE_SIZE)) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	if (tdi_index >= mmgr->tdi_context_entries_count) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_INTERFACE;
	}

	status = mmgr->dmb->map_soc_address (mmgr->dmb, mmgr->tdi_context_soc_address,
		sizeof (struct tdisp_tdi_context) * mmgr->tdi_context_entries_count, HSP_DMB_ACCESS_WRITE,
		(void**) &tdi_contexts);

	if (status != 0) {
		return status;
	}

	status = tdisp_tdi_context_set_start_nonce (&tdi_contexts[tdi_index], nonce, nonce_size);

	mmgr->dmb->unmap_soc_address (mmgr->dmb, tdi_contexts);

	return status;
}

int tdisp_tdi_context_manager_manticore_set_lock_flags (const struct tdisp_tdi_context_manager *mgr,
	uint32_t tdi_index, uint16_t lock_flags)
{
	const struct tdisp_tdi_context_manager_manticore *mmgr = TO_DERIVED_TYPE (mgr,
		const struct tdisp_tdi_context_manager_manticore, base);
	struct tdisp_tdi_context *tdi_contexts = NULL;
	int status;

	if (mgr == NULL) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	if (tdi_index >= mmgr->tdi_context_entries_count) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_INTERFACE;
	}

	status = mmgr->dmb->map_soc_address (mmgr->dmb, mmgr->tdi_context_soc_address,
		sizeof (struct tdisp_tdi_context) * mmgr->tdi_context_entries_count, HSP_DMB_ACCESS_WRITE,
		(void**) &tdi_contexts);

	if (status != 0) {
		return status;
	}

	status = tdisp_tdi_context_set_lock_flags (&tdi_contexts[tdi_index], lock_flags);

	mmgr->dmb->unmap_soc_address (mmgr->dmb, tdi_contexts);

	return status;
}

int tdisp_tdi_context_manager_manticore_set_default_ide_stream (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_index, uint8_t ide_stream_id)
{
	const struct tdisp_tdi_context_manager_manticore *mmgr = TO_DERIVED_TYPE (mgr,
		const struct tdisp_tdi_context_manager_manticore, base);
	struct tdisp_tdi_context *tdi_contexts = NULL;
	int status;

	if (mgr == NULL) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	if (tdi_index >= mmgr->tdi_context_entries_count) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_INTERFACE;
	}

	status = mmgr->dmb->map_soc_address (mmgr->dmb, mmgr->tdi_context_soc_address,
		sizeof (struct tdisp_tdi_context) * mmgr->tdi_context_entries_count, HSP_DMB_ACCESS_WRITE,
		(void**) &tdi_contexts);

	if (status != 0) {
		return status;
	}

	status = tdisp_tdi_context_set_default_ide_stream (&tdi_contexts[tdi_index], ide_stream_id);

	mmgr->dmb->unmap_soc_address (mmgr->dmb, tdi_contexts);

	return status;
}

int tdisp_tdi_context_manager_manticore_set_mmio_reporting_offset (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_index, uint64_t mmio_reporting_offset)
{
	const struct tdisp_tdi_context_manager_manticore *mmgr = TO_DERIVED_TYPE (mgr,
		const struct tdisp_tdi_context_manager_manticore, base);
	struct tdisp_tdi_context *tdi_contexts = NULL;
	int status;

	if (mgr == NULL) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	if (tdi_index >= mmgr->tdi_context_entries_count) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_INTERFACE;
	}

	status = mmgr->dmb->map_soc_address (mmgr->dmb, mmgr->tdi_context_soc_address,
		sizeof (struct tdisp_tdi_context) * mmgr->tdi_context_entries_count, HSP_DMB_ACCESS_WRITE,
		(void**) &tdi_contexts);

	if (status != 0) {
		return status;
	}

	status = tdisp_tdi_context_set_mmio_reporting_offset (&tdi_contexts[tdi_index],
		mmio_reporting_offset);

	mmgr->dmb->unmap_soc_address (mmgr->dmb, tdi_contexts);

	return status;
}

int tdisp_tdi_context_manager_manticore_set_bind_p2p_address_mask (
	const struct tdisp_tdi_context_manager *mgr, uint32_t tdi_index, uint64_t bind_p2p_address_mask)
{
	const struct tdisp_tdi_context_manager_manticore *mmgr = TO_DERIVED_TYPE (mgr,
		const struct tdisp_tdi_context_manager_manticore, base);
	struct tdisp_tdi_context *tdi_contexts = NULL;
	int status;

	if (mgr == NULL) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	if (tdi_index >= mmgr->tdi_context_entries_count) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_INTERFACE;
	}

	status = mmgr->dmb->map_soc_address (mmgr->dmb, mmgr->tdi_context_soc_address,
		sizeof (struct tdisp_tdi_context) * mmgr->tdi_context_entries_count, HSP_DMB_ACCESS_WRITE,
		(void**) &tdi_contexts);

	if (status != 0) {
		return status;
	}

	status = tdisp_tdi_context_set_bind_p2p_address_mask (&tdi_contexts[tdi_index],
		bind_p2p_address_mask);

	mmgr->dmb->unmap_soc_address (mmgr->dmb, tdi_contexts);

	return status;
}

int tdisp_tdi_context_manager_manticore_set_reserved (const struct tdisp_tdi_context_manager *mgr,
	uint32_t tdi_index, uint8_t index, uint32_t value)
{
	const struct tdisp_tdi_context_manager_manticore *mmgr = TO_DERIVED_TYPE (mgr,
		const struct tdisp_tdi_context_manager_manticore, base);
	struct tdisp_tdi_context *tdi_contexts = NULL;
	int status;

	if ((mgr == NULL) || (index >= ARRAY_SIZE (tdi_contexts[0].reserved))) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	if (tdi_index >= mmgr->tdi_context_entries_count) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_INTERFACE;
	}

	status = mmgr->dmb->map_soc_address (mmgr->dmb, mmgr->tdi_context_soc_address,
		sizeof (struct tdisp_tdi_context) * mmgr->tdi_context_entries_count, HSP_DMB_ACCESS_WRITE,
		(void**) &tdi_contexts);

	if (status != 0) {
		return status;
	}

	status = tdisp_tdi_context_set_reserved (&tdi_contexts[tdi_index], index, value);

	mmgr->dmb->unmap_soc_address (mmgr->dmb, tdi_contexts);

	return status;
}

/**
 * Runtime initialization of Manticore TDISP TDI context manager
 *
 * @param mgr Instance to initialize
 * @param tdi_context_soc_address SOC address for TDI context
 * @param tdi_context_entries_count Number of TDI context entries
 * @param dmb DMB instance to use for accessing TDI context
 *
 * @return 0 on success, error code otherwise
 */
int tdisp_tdi_context_manager_manticore_init (struct tdisp_tdi_context_manager_manticore *mgr,
	uint64_t tdi_context_soc_address, size_t tdi_context_entries_count, const struct hsp_dmb *dmb)
{
	if ((mgr == NULL) || (tdi_context_entries_count == 0) || (dmb == NULL)) {
		return TDISP_TDI_CONTEXT_MANAGER_INVALID_ARGUMENT;
	}

	memset (mgr, 0, sizeof (*mgr));

	mgr->base.clear_tdi_context = tdisp_tdi_context_manager_manticore_clear_tdi_context;
	mgr->base.clear_all_tdi_context = tdisp_tdi_context_manager_manticore_clear_all_tdi_context;
	mgr->base.get_tdi_context = tdisp_tdi_context_manager_manticore_get_tdi_context;
	mgr->base.set_start_nonce = tdisp_tdi_context_manager_manticore_set_start_nonce;
	mgr->base.set_lock_flags = tdisp_tdi_context_manager_manticore_set_lock_flags;
	mgr->base.set_mmio_reporting_offset =
		tdisp_tdi_context_manager_manticore_set_mmio_reporting_offset;
	mgr->base.set_default_ide_stream = tdisp_tdi_context_manager_manticore_set_default_ide_stream;
	mgr->base.set_bind_p2p_address_mask =
		tdisp_tdi_context_manager_manticore_set_bind_p2p_address_mask;
	mgr->base.set_reserved = tdisp_tdi_context_manager_manticore_set_reserved;

	mgr->tdi_context_soc_address = tdi_context_soc_address;
	mgr->tdi_context_entries_count = tdi_context_entries_count;
	mgr->dmb = dmb;

	return 0;
}

/**
 * Cleanup Manticore TDISP TDI context manager
 *
 * @param mgr TDI context manager instance
 */
void tdisp_tdi_context_manager_manticore_release (
	const struct tdisp_tdi_context_manager_manticore *mgr)
{
	UNUSED (mgr);
}
