// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef GRACEFUL_SHUTDOWN_STATIC_H_
#define GRACEFUL_SHUTDOWN_STATIC_H_

#include "graceful_shutdown.h"


/* Internal functions declared to allow for static initialization. */
int graceful_shutdown_get_uuid (const struct cmd_device *device, uint8_t *buffer, size_t buf_len);
int graceful_shutdown_reset (const struct cmd_device *device);
int graceful_shutdown_get_reset_counter (const struct cmd_device *device, uint8_t type,
	uint8_t port, uint16_t *counter);
int graceful_shutdown_get_heap_stats (const struct cmd_device *device,
	struct cmd_device_heap_stats *heap);

int graceful_shutdown_get_allowed_drain_time (const struct graceful_shutdown_control *handler,
	uint32_t *drain_time_ms);
int graceful_shutdown_set_allowed_drain_time (const struct graceful_shutdown_control *handler,
	uint32_t drain_time_ms);


/**
 * Constant initializer for the shutdown control API.
 */
#define	GRACEFUL_SHUTDOWN_CONTROL_API_INIT	{ \
		.get_allowed_drain_time = graceful_shutdown_get_allowed_drain_time, \
		.set_allowed_drain_time = graceful_shutdown_set_allowed_drain_time, \
	}

/**
 * Constant initializer for the heap stats request.
 */
#ifdef CMD_ENABLE_HEAP_STATS
#define	GRACEFUL_SHUTDOWN_HEAP_STATS_API\
	.get_heap_stats = graceful_shutdown_get_heap_stats,
#else
#define	GRACEFUL_SHUTDOWN_HEAP_STATS_API
#endif

/**
 * Constant initializer for the device command API.
 */
#define	GRACEFUL_SHUTDOWN_DEVICE_API_INIT	{ \
		.get_uuid = graceful_shutdown_get_uuid, \
		.reset = graceful_shutdown_reset, \
		.get_reset_counter = graceful_shutdown_get_reset_counter, \
		GRACEFUL_SHUTDOWN_HEAP_STATS_API \
	}


/**
 * Initialize a static handler for coordinating a graceful shutdown of CP and FP cores.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the handler.
 * @param device_ptr Underlying handler for device commands.  This will be used to actually trigger
 * the reset once the shutdown has completed.
 * @param ipc_ptr Channel to use for sending IPC requests to the CP Admin core.
 * @param log_flush_ptr Log flusher to use for flushing logs before the shutdown.
 * @param shutdown_indicator_ptr Reset tolerant storage for the magic number indicating a graceful
 * shutdown and reset.
 * @param shutdown_timeout_ms_arg The timeout to use when sending shutdown IPC messages, in
 * milliseconds.  This will be added to the configured drain time to determine how long to wait.
 * @param drain_time_ms_arg The initial value to configure for the allowed drain time.  This will
 * also serve as the default value to use, if requested.
 */
#define	graceful_shutdown_static_init(state_ptr, device_ptr, ipc_ptr, log_flush_ptr, \
	shutdown_indicator_ptr, shutdown_timeout_ms_arg, drain_time_ms) { \
		.base_ctrl = GRACEFUL_SHUTDOWN_CONTROL_API_INIT, \
		.base_device = GRACEFUL_SHUTDOWN_DEVICE_API_INIT, \
		.state = state_ptr, \
		.device = device_ptr, \
		.ipc = ipc_ptr, \
		.log_flush = log_flush_ptr, \
		.indicator = shutdown_indicator_ptr, \
		.ipc_timeout = shutdown_timeout_ms_arg, \
		.default_drain_time = drain_time_ms, \
	}


#endif	/* GRACEFUL_SHUTDOWN_STATIC_H_ */
