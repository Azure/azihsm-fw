// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_ERROR_STATE_H_
#define MANTICORE_ERROR_STATE_H_

#include "platform_api.h"
#include "cmd_interface/cmd_device.h"
#include "crashdump/soc_crashdump_interface.h"
#include "fips/error_state_entry_interface.h"
#include "logging/log_flush_handler.h"
#include "system/periodic_task.h"


/**
 * Variable context for the error state handler.
 */
struct manticore_error_state_state {
	platform_semaphore error_event;			/**< Signal than an error has occurred. */
	struct debug_log_entry_info log_task;	/**< Cache for the error log details to store from task context. */
	struct debug_log_entry_info log_isr;	/**< Cache for the error log details to store from ISR context. */
};

/**
 * Handler for the FIPS error state on Manticore.
 */
struct manticore_error_state {
	struct periodic_task_handler base_task;					/**< Base API for periodic task integration. */
	struct error_state_entry_interface base_error_task;		/**< Base API for error state entry from task context */
	struct error_state_entry_interface base_error_isr;		/**< Base API for error state entry form ISR context. */
	struct manticore_error_state_state *state;				/**< Variable context for error state handling. */
	const struct error_state_entry_interface *hsp_entry;	/**< Handler for putting the HSP into the error state. */
	const struct soc_crashdump_interface *soc_entry;		/**< Interface for putting the SoC cores into the error state. */
	const struct log_flush_handler *soc_log;				/**< Debug log handler for the SoC cores. */
	const struct cmd_device *handle_error;					/**< Interface to handle the error state via a device reset. */
};


int manticore_error_state_init (struct manticore_error_state *handler,
	struct manticore_error_state_state *state, const struct error_state_entry_interface *hsp_entry,
	const struct soc_crashdump_interface *soc_entry, const struct log_flush_handler *soc_log,
	const struct cmd_device *handle_error);
int manticore_error_state_init_state (const struct manticore_error_state *handler);
void manticore_error_state_release (const struct manticore_error_state *handler);


#endif	/* MANTICORE_ERROR_STATE_H_ */
