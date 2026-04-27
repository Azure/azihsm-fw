// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_CRASHDUMP_HANDLER_STATIC_H_
#define SOC_CRASHDUMP_HANDLER_STATIC_H_

#include "soc_crashdump_handler.h"
#include "soc_crashdump_interface.h"


/* Internal functions declared to allow for static initialization. */
void soc_crashdump_handler_task_handler_prepare (const struct periodic_task_handler *handler);
const platform_clock* soc_crashdump_handler_task_handler_next_execution (
	const struct periodic_task_handler *task_handler);
void soc_crashdump_handler_task_handler_execute (const struct periodic_task_handler *task_handler);


/**
 * Constant initializer for the crashdump monitor task handlers.
 */
#define	SOC_CRASHDUMP_HANDLER_TASK_HANDLERS  { \
		.prepare = soc_crashdump_handler_task_handler_prepare, \
		.get_next_execution = soc_crashdump_handler_task_handler_next_execution, \
		.execute = soc_crashdump_handler_task_handler_execute \
	}

/**
 * Initialize a static instance of a crashdump.
 *
 * There is no validation done on the arguments
 * @param[in] state_ptr Variable context for the crash dump handler.
 * @param[in] refresh_t The amount of time between calls to execute SoC crashdump handler.
 * @param[in] soc_api_ptr Crashdump SoC interface to make SoC access for crashdump operations.
 * @param[in] log_flush_ptr Log flusher used to flush the log buffers before crashdump.
 * @param[in] fw_version_ptr The build version number for the firmware package.
 * @param[in] fw_version_length The length of build version number for the firmware package.
 */
#define soc_crashdump_handler_static_init(state_ptr, refresh_t, soc_api_ptr, log_flush_ptr, fw_version_ptr, fw_version_length) { \
	.base = SOC_CRASHDUMP_HANDLER_TASK_HANDLERS, \
	.state = state_ptr, \
	.refresh_period = refresh_t, \
	.soc_api = soc_api_ptr, \
	.log_flush = log_flush_ptr, \
	.fw_version = fw_version_ptr, \
	.fw_version_len = fw_version_length, \
	}


#endif	/* SOC_CRASHDUMP_HANDLER_STATIC_H_ */
