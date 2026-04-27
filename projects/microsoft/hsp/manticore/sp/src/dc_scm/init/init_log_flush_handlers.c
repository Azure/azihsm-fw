// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include "init_log.h"
#include "init_log_flush_handlers.h"
#include "init_system.h"
#include "common/array_size.h"
#include "dc_scm/rot_memory_map.h"
#include "logging/init_logging.h"
#include "logging/log_flush_handler_static.h"
#include "logging/logging_collector.h"
#include "logging/logging_collector_static.h"
#include "logging/msft_debug_log.h"


/**
 * Expected number of records in the CP cores logging ring_buffers.
 */
#define CP_LOG_COLLECTOR_EXPECTED_RECORDS 170

/**
 * Expected number of records in the FP cores logging ring_buffers.
 */
#define FP_LOG_COLLECTOR_EXPECTED_RECORDS 336


/**
 * CP0 GSRAM logging ring_buffer
 */
static struct logging_collector_state logging_collector_state_cp0;
static const struct logging_collector logging_collector_cp0 =
	logging_collector_static_init (&logging_collector_state_cp0, CP0_DEBUG_LOG_RING_BUFFER,
	CP_LOG_COLLECTOR_EXPECTED_RECORDS, &dmb, MSFT_LOGGING_COMPONENT_MANTICORE_CP0);

/**
 * CP1 GSRAM logging ring_buffer
 */
static struct logging_collector_state logging_collector_state_cp1;
static const struct logging_collector logging_collector_cp1 =
	logging_collector_static_init (&logging_collector_state_cp1, CP1_DEBUG_LOG_RING_BUFFER,
	CP_LOG_COLLECTOR_EXPECTED_RECORDS, &dmb, MSFT_LOGGING_COMPONENT_MANTICORE_CP1);

/**
 * FP0 PSRAM logging ring_buffer
 */
static struct logging_collector_state logging_collector_state_fp0;
static const struct logging_collector logging_collector_fp0 =
	logging_collector_static_init (&logging_collector_state_fp0, FP0_DEBUG_LOG_RING_BUFFER,
	FP_LOG_COLLECTOR_EXPECTED_RECORDS, &dmb, MSFT_LOGGING_COMPONENT_MANTICORE_FP0);

/**
 * FP1 PSRAM logging ring_buffer
 */
static struct logging_collector_state logging_collector_state_fp1;
static const struct logging_collector logging_collector_fp1 =
	logging_collector_static_init (&logging_collector_state_fp1, FP1_DEBUG_LOG_RING_BUFFER,
	FP_LOG_COLLECTOR_EXPECTED_RECORDS, &dmb, MSFT_LOGGING_COMPONENT_MANTICORE_FP1);

/**
 * FP2 PSRAM logging ring_buffer
 */
static struct logging_collector_state logging_collector_state_fp2;
static const struct logging_collector logging_collector_fp2 =
	logging_collector_static_init (&logging_collector_state_fp2, FP2_DEBUG_LOG_RING_BUFFER,
	FP_LOG_COLLECTOR_EXPECTED_RECORDS, &dmb, MSFT_LOGGING_COMPONENT_MANTICORE_FP2);

/**
 * List of logs to flush to flash.
 */
static const struct logging *const flush_list[] = {
	&logging_collector_cp0.base,
	&logging_collector_cp1.base,
	&logging_collector_fp0.base,
	&logging_collector_fp1.base,
	&logging_collector_fp2.base,
	/* Keep the flash-based debug_logger last in order to flush the collectors as well. */
	&debug_logger.base,
};

/**
 * Variable context for storing the debug log to flash.
 */
static struct log_flush_handler_state log_flush_context;

/**
 * Handler for storing the debug log to flash.
 */
const struct log_flush_handler log_flush = log_flush_handler_static_init (&log_flush_context,
	flush_list, ARRAY_SIZE (flush_list), 1000);


/**
 * Initialize the log flush handlers.
 *
 * @return 0 if the handlers were successfully initialized or an error code.
 */
int initialize_log_flush_handlers ()
{
	int status;

	status = logging_collector_init_state (&logging_collector_cp0);
	if (status != 0) {
		return status;
	}

	status = logging_collector_init_state (&logging_collector_cp1);
	if (status != 0) {
		return status;
	}

	status = logging_collector_init_state (&logging_collector_fp0);
	if (status != 0) {
		return status;
	}

	status = logging_collector_init_state (&logging_collector_fp1);
	if (status != 0) {
		return status;
	}

	status = logging_collector_init_state (&logging_collector_fp2);
	if (status != 0) {
		return status;
	}

	status = log_flush_handler_init_state (&log_flush);

	return status;
}
