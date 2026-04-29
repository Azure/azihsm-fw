// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_CRASHDUMP_MBX_ERR_HANDLER_H_
#define SOC_CRASHDUMP_MBX_ERR_HANDLER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "hsp_top.h"
#include "fips/error_state_entry_interface.h"
#include "mmio/mmio_register_block.h"
#include "trap/hsp_interrupt_handler.h"


/**
 * Mailbox error handler structure.
 */
struct soc_crashdump_mbx_err_handler {
	struct hsp_interrupt_handler base;					/**< hsp_interrupt_handler object. */
	const struct error_state_entry_interface *error;	/**< Error state management interface */
	const struct mmio_register_block *creg;				/**< Register interface for a struct Creg_regs mapping.  */
	size_t mbx_regs_offset;								/**< CREG offset for the MBX crypto registers. */
};


int soc_crashdump_mbx_err_handler_init (struct soc_crashdump_mbx_err_handler *handler,
	const struct error_state_entry_interface *error, const struct mmio_register_block *creg,
	size_t mbx_regs_offset);
void soc_crashdump_mbx_err_handler_release (
	const struct soc_crashdump_mbx_err_handler *handler);


#endif	/* SOC_CRASHDUMP_MBX_ERR_HANDLER_H_ */
