// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_ERROR_STATE_STATIC_H_
#define MANTICORE_ERROR_STATE_STATIC_H_

#include "manticore_error_state.h"


/* Internal functions declared to allow for static initialization. */
const platform_clock* manticore_error_state_get_next_execution (
	const struct periodic_task_handler *handler);
void manticore_error_state_execute (const struct periodic_task_handler *handler);

void manticore_error_state_enter_error_state_from_task (
	const struct error_state_entry_interface *entry, const struct debug_log_entry_info *error_log);
void manticore_error_state_enter_error_state_from_isr (
	const struct error_state_entry_interface *entry, const struct debug_log_entry_info *error_log);


/**
 * Constant initializer for the task handler API.
 */
#define	MANTICORE_ERROR_STATE_TASK_API_INIT  { \
		.prepare = NULL, \
		.get_next_execution = manticore_error_state_get_next_execution, \
		.execute = manticore_error_state_execute, \
	}

/**
 * Constant initializer for the error state entry API from task context.
 */
#define	MANTICORE_ERROR_STATE_TASK_ENTRY_API_INIT  { \
		.enter_error_state = manticore_error_state_enter_error_state_from_task, \
	}

/**
 * Constant initializer for the error state entry API from interrupt context.
 */
#define	MANTICORE_ERROR_STATE_ISR_ENTRY_API_INIT  { \
		.enter_error_state = manticore_error_state_enter_error_state_from_isr, \
	}


/**
 * Initialize a static instance of the FIPS error state handler for Manticore.  This can be a
 * constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the error state handler.
 * @param hsp_entry_ptr Error state entry handler for HSP.
 * @param soc_entry_ptr Error state entry handler for the rest of the SoC.
 * @param soc_log_ptr Debug log handler for SoC cores.
 * @param handle_error_ptr Interface to resolve the error state through a device reset.
 */
#define	manticore_error_state_static_init(state_ptr, hsp_entry_ptr, soc_entry_ptr, soc_log_ptr, \
	handle_error_ptr)	{ \
		.base_task = MANTICORE_ERROR_STATE_TASK_API_INIT, \
		.base_error_task = MANTICORE_ERROR_STATE_TASK_ENTRY_API_INIT, \
		.base_error_isr = MANTICORE_ERROR_STATE_ISR_ENTRY_API_INIT, \
		.state = state_ptr, \
		.hsp_entry = hsp_entry_ptr, \
		.soc_entry = soc_entry_ptr, \
		.soc_log = soc_log_ptr, \
		.handle_error = handle_error_ptr, \
	}


#endif	/* MANTICORE_ERROR_STATE_STATIC_H_ */
