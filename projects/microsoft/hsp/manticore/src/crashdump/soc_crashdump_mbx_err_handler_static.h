// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_CRASHDUMP_MBX_ERR_HANDLER_STATIC_H_
#define SOC_CRASHDUMP_MBX_ERR_HANDLER_STATIC_H_

#include "soc_crashdump_mbx_err_handler.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */
bool soc_crashdump_handle_mbx_err_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param);

/**
 * Initialize a static instance of a crashdump MBX error handling.
 *
 * There is no validation done on the arguments
 * @param[in] error_entry The error state entry interface pointer.
 * @param[in] creg_regs The Creg register pointer.
 * @param[in] mbx_regs_set The mailbox register set offset.
 */
#define soc_crashdump_mbx_err_handler_static_init(error_entry, creg_regs, mbx_regs_set) { \
	.base = hsp_interrupt_handler_static_init(soc_crashdump_handle_mbx_err_interrupt), \
	.error = error_entry, \
	.creg = creg_regs, \
	.mbx_regs_offset = mbx_regs_set, \
}


#endif	/* SOC_CRASHDUMP_MBX_ERR_HANDLER_STATIC_H_ */
