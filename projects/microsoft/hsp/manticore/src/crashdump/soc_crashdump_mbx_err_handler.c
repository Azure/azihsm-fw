// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "common/type_cast.h"
#include "common/unused.h"
#include "crashdump/soc_crashdump_mbx_err_handler.h"
#include "logging/manticore_logging.h"

bool soc_crashdump_handle_mbx_err_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	int status;
	const struct soc_crashdump_mbx_err_handler *mbx_handler = TO_DERIVED_TYPE (handler,
		const struct soc_crashdump_mbx_err_handler, base);

	UNUSED (param);

	/* Clear the pending interrupt from creg. */
	status = mbx_handler->creg->map (mbx_handler->creg);
	if (status != 0) {
		goto error;
	}

	/* Clear the interrupt status register. */
	status = mbx_handler->creg->write32 (mbx_handler->creg,
		mbx_handler->mbx_regs_offset + CREG_REGS_SYS_MBX_S2H_MBX_INSTS_OFFSET,
		CREG_REGS_SYS_MBX_S2H_MBX_INSTS_ERR_BIT_FIELD_MASK);
	if (status != 0) {
		goto unmap;
	}

unmap:
	mbx_handler->creg->unmap (mbx_handler->creg);

error:
	/* Notify the error state handler. No log information required as it trigger by other core */
	mbx_handler->error->enter_error_state (mbx_handler->error, NULL);

	return (status == 0) ? true : false;
}

/**
 * Initialize crashdump SOC MBX error interrupt handler.
 *
 * @param[in] handler The crashdump SOC MBX error interrupt handler to initialize.
 * @param[in] error The error state entry interface pointer.
 * @param[in] creg The The CREG register interface (struct Creg_regs).
 * @param[in] mbx_regs_offset The mailbox register set offset.
 *
 * @return 0 if the interrupt handler was successfully initialized or an error code.
 */
int soc_crashdump_mbx_err_handler_init (struct soc_crashdump_mbx_err_handler *handler,
	const struct error_state_entry_interface *error, const struct mmio_register_block *creg,
	size_t mbx_regs_offset)
{
	if ((handler == NULL) || (error == NULL) || (creg == NULL)) {
		return MANTICORE_LOGGING_HW_ERR_ENABLE_MBX_INT_FAILED;
	}

	handler->base.handle_interrupt = soc_crashdump_handle_mbx_err_interrupt;
	handler->error = error;
	handler->creg = creg;
	handler->mbx_regs_offset = mbx_regs_offset;

	return 0;
}

/**
 * Release the resources used for crashdump SOC MBX error interrupt handler.
 *
 * @param[in] handler The crashdump SOC MBX error interrupt handler to release.
 */
void soc_crashdump_mbx_err_handler_release (
	const struct soc_crashdump_mbx_err_handler *handler)
{
	UNUSED (handler);
}
