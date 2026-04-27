// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_crashdump_hw_err_handler_mem_err.h"
#include "common/type_cast.h"
#include "crashdump/hsp_crashdump_packet.h"
#include "logging/debug_log.h"
#include "logging/hsp_logging.h"


bool hsp_crashdump_hw_err_handler_mem_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	const struct hsp_crashdump_hw_err_handler_mem_err *mem_err_handler =
		TO_DERIVED_TYPE (TO_DERIVED_TYPE (handler, const struct hsp_crashdump_hw_err_handler, base),
		const struct hsp_crashdump_hw_err_handler_mem_err, base);
	int status;
	uint32_t intsts_value = 0;
	uint32_t err_reg_offset = mem_err_handler->mem_regs +
		CREG_REGS_MEM_ERR_MEM_PKAR1_ERR_ADDR_OFFSET;
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data hw_err_data = {0, 0, 0, 0};

	status = mem_err_handler->base.creg->map (mem_err_handler->base.creg);
	if (status != 0) {
		goto skip_unmap;
	}

	/* Get interrupt status. */
	status = mem_err_handler->base.creg->read32 (mem_err_handler->base.creg,
		mem_err_handler->mem_regs + CREG_REGS_MEM_ERR_MEM_ERR_INTSTS_OFFSET, &intsts_value);
	if (status != 0) {
		goto failed;
	}

	if (!intsts_value) {
		status = HSP_CRASHDUMP_HW_ERR_HANDLER_MEM_ERR_INT_STATUS_0;
		goto failed;
	}

	hw_err_data.int_status = (uint16_t) (intsts_value & 0xFFFF);

	/* Get the location where the error occurred. */
	for (hw_err_data.err_tag = 0;
		hw_err_data.err_tag < HSP_CRASHDUMP_HW_ERR_HANDLER_MEM_ERR_NUM_ERR_REGS;
		hw_err_data.err_tag++) {
		status = mem_err_handler->base.creg->read32 (mem_err_handler->base.creg, err_reg_offset,
			&hw_err_data.status0);
		if (status != 0) {
			goto failed;
		}

		if (hw_err_data.status0) {
			hw_err_data.err_tag++;
			break;
		}

		if (hw_err_data.err_tag == 3) {
			err_reg_offset += sizeof (uint8_t) * 0x14;
		}
		else {
			err_reg_offset += sizeof (uint32_t);
		}
	}

	/* Clear the pending interrupt from memory module. */
	status = mem_err_handler->base.creg->write32 (mem_err_handler->base.creg,
		mem_err_handler->mem_regs + CREG_REGS_MEM_ERR_MEM_ERR_INTSTS_OFFSET,
		CREG_REGS_MEM_ERR_MEM_ERR_INTSTS_WRITE_MASK);

failed:
	mem_err_handler->base.creg->unmap (mem_err_handler->base.creg);

skip_unmap:
	if (status != 0) {
		/* Return false, to cease the interrupt dispatch. */
		return false;
	}

	return hsp_crashdump_hw_err_handler_collect_hsp_crashdump (
		mem_err_handler->base.crashdump_handler, param, HSP_CRASHDUMP_PACKET_FAULT_CODE_MEM_ERR,
		&hw_err_data);
}

/**
 * Save HW error opaque data to debug log entries.
 *
 * @param[in] buffer The buffer where crashdump opaque data is stored.
 * @param[in] length The number bytes of crashdump opaque data.
 *
 */
void hsp_crashdump_hw_err_handler_mem_err_save_opaque_data (uint32_t *buffer, size_t length)
{
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data *hw_err_data =
		(struct hsp_crashdump_hw_err_handler_crashdump_opaque_data*) buffer;

	/* TODO: Any additional opaque data in the crash dump packet will not get logged. */
	if ((hw_err_data != NULL) &&
		(length >= sizeof (struct hsp_crashdump_hw_err_handler_crashdump_opaque_data))) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_MEM, HSP_LOGGING_HW_ERROR_MEM_INTSTS_TAG, hw_err_data->int_status);
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_MEM, hw_err_data->err_tag, hw_err_data->status0);
	}
}

/**
 * Initialize crashdump HSP HW error memory error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error memory error interrupt handler to initialize.
 * @param[in] crashdump_handler The HSP crashdump handler.
 * @param[in] creg The The CREG register interface (struct Creg_regs).
 * @param[in] mem_regs The CREG offset for the hardware memory interrupt registers.
 *
 * @return 0 if the interrupt handler was successfully initialized or an error code.
 */
int hsp_crashdump_hw_err_handler_mem_err_init (
	struct hsp_crashdump_hw_err_handler_mem_err *handler,
	const struct hsp_crashdump_handler *crashdump_handler, const struct mmio_register_block *creg,
	size_t mem_regs)
{
	int status;

	if ((handler == NULL) || (crashdump_handler == NULL) || (creg == NULL)) {
		return HSP_CRASHDUMP_HW_ERR_HANDLER_MEM_ERR_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct hsp_crashdump_hw_err_handler_mem_err));

	status = hsp_crashdump_hw_err_handler_init (&handler->base, crashdump_handler, creg,
		hsp_crashdump_hw_err_handler_mem_err);

	if (status != 0) {
		return status;
	}

	handler->mem_regs = mem_regs;

	return 0;
}

/**
 * Release the resources used for crashdump HSP HW error memory error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error memory error interrupt handler to release.
 */
void hsp_crashdump_hw_err_handler_mem_err_release (
	const struct hsp_crashdump_hw_err_handler_mem_err *handler)
{
	hsp_crashdump_hw_err_handler_release (&handler->base);
}
