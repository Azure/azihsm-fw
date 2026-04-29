// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_CRASHDUMP_HANDLER_H_
#define SOC_CRASHDUMP_HANDLER_H_

#include <stdbool.h>
#include <stdint.h>
#include "soc_crashdump_interface.h"
#include "crashdump/hsp_crashdump_handler.h"
#include "logging/log_flush_handler.h"
#include "splibs/inc/sptypes.h"
#include "status/module_id.h"
#include "system/periodic_task.h"


/* TODO: This flag and code guarded by this flag should be removed after
 * crashdump code gets stable. */
#define SOC_CRASHDUMP_HANDLER_PRINTF					1
#define SOC_CRASHDUMP_HANDLER_PRINTF_OFF_BY_DEFAULT		0

/* The amount of time (ms) to delay to start SoC crashdump handler task execute. */
#define SOC_CRASHDUMP_HANDLER_EXE_START_DELAY		3000

/**
 * Variable context for the crashdump handler.
 */
struct soc_crashdump_handler_state {
	platform_clock next;		/**< Time for the next execution. */
	bool first_check;			/**< True if this is the first time we are checking core liveness. */
	bool unrecoverable_fault;	/**< True if the system is in an unrecoverable fault state. */
	bool run_monitor;			/**< True when the SoC crash monitor is active. */
};

/**
 * Crashdump implementation to provide crashdump functions.
 */
struct soc_crashdump_handler {
	struct periodic_task_handler base;				/**< Task handler for crashdump monitor task. */
	const struct soc_crashdump_interface *soc_api;	/**< Crashdump SoC interface to make SoC access for crashdump operations. */
	const struct log_flush_handler *log_flush;		/**< Log flush handler to flush logs. */
	struct soc_crashdump_handler_state *state;		/**< Variable context for the handler. */
	uint32_t refresh_period;						/**< The amount of time between calls to execute SoC crashdump handler. */
	const uint8_t *fw_version;						/**< The build version number for the firmware package. */
	size_t fw_version_len;							/**< The length of build version number for the firmware package. */
};


int soc_crashdump_handler_init (struct soc_crashdump_handler *handler,
	struct soc_crashdump_handler_state *state, const uint32_t refresh_period,
	const struct soc_crashdump_interface *soc_api, const struct log_flush_handler *log_flush,
	uint8_t *fw_version, size_t fw_version_len);
void soc_crashdump_handler_release (const struct soc_crashdump_handler *handler);

int soc_crashdump_handler_get_crashdumps (const struct soc_crashdump_handler *handler,
	uint32_t *num_available, enum soc_crashdump_arm_core_id *failed_core_id);
void soc_crashdump_handler_start_crash_monitor (const struct soc_crashdump_handler *handler);


#endif	/* SOC_CRASHDUMP_HANDLER_H_ */
