// Copyright (c) Microsoft Corporation. All rights reserved.

#include "init_ephemeral_key.h"
#include "init_ephemeral_key_monitor.h"
#include "init_system.h"
#include "periodic_task_freertos_static.h"
#include "task_log_id.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "common/array_size.h"
#include "keystore/ephemeral_key_monitor_static.h"


/**
 * Period between ephemeral key monitor scan ticks, in milliseconds.
 */
#define	EPHEMERAL_KEY_MONITOR_TASK_INTERVAL_MS		500

/* The monitor's view of the persistent-store geometry must exactly cover the GSRAM region the CP
 * HSM reserves for it; otherwise per-PFN slot addressing would run off the end of the region. */
_Static_assert (PART_PERSISTENT_STORE_SLOT_SIZE * PART_PERSISTENT_STORE_SLOT_COUNT ==
	GSRAM_MEM_MAP_HSM_PART_PERSISTENT_STORE_SIZE,
	"persistent-store slot geometry does not match the reserved GSRAM region size");


/**
 * Ephemeral key monitor state.
 */
static struct ephemeral_key_monitor_state rsa_ephemeral_key_monitor_state;

/**
 * Working buffer for the ephemeral key monitor (DER key in, PKA-LE payload out).  Held as a CSP in
 * static storage so its zeroization survives task preemption.
 */
static uint8_t rsa_ephemeral_key_monitor_key_buffer[EPHEMERAL_KEY_MONITOR_KEY_BUFFER_SIZE];

/**
 * Ephemeral key monitor that scans the HSM partition persistent store and
 * refills empty unwrapping-key slots from the flash key cache.
 */
const struct ephemeral_key_monitor rsa_ephemeral_key_monitor =
	ephemeral_key_monitor_static_init (&rsa_ephemeral_key_monitor_state, &dmb,
	&rsa_key_cache_flash.base, rsa_ephemeral_key_monitor_key_buffer,
	sizeof (rsa_ephemeral_key_monitor_key_buffer), EPHEMERAL_KEY_MONITOR_TASK_INTERVAL_MS,
	(uint64_t) HSM_PART_PERSISTENT_STORE_ADDRESS);

/**
 * Periodic task state for the ephemeral key monitor.
 */
static struct periodic_task_freertos_state rsa_ephemeral_key_monitor_task_context;

/**
 * Periodic task handlers array for the ephemeral key monitor.
 */
static const struct periodic_task_handler *const ephemeral_key_monitor_handlers[] = {
	&rsa_ephemeral_key_monitor.base,
};

/**
 * Task that runs the ephemeral key monitor.
 */
static const struct periodic_task_freertos ephemeral_key_monitor_task =
	periodic_task_freertos_static_init (&rsa_ephemeral_key_monitor_task_context,
	ephemeral_key_monitor_handlers, ARRAY_SIZE (ephemeral_key_monitor_handlers),
	EPHEMERAL_KEY_MONITOR_TASK_LOG_ID);

/**
 * Statically allocated task control block for the ephemeral key monitor task.
 */
static StaticTask_t ephemeral_key_monitor_task_tcb;

/**
 * Statically allocated stack for the ephemeral key monitor task.
 */
static StackType_t ephemeral_key_monitor_task_stack[EPHEMERAL_KEY_MONITOR_TASK_STACK_WORDS];


/**
 * Initialize the ephemeral key monitor infrastructure.
 *
 * @return 0 if successful or an error code.
 */
int initialize_ephemeral_key_monitor ()
{
	return ephemeral_key_monitor_init_state (&rsa_ephemeral_key_monitor);
}

/**
 * Start the task handler for the ephemeral key monitor.
 *
 * @return 0 if the task handler was started or an error code.
 */
int start_ephemeral_key_monitor ()
{
	int status;

	status = periodic_task_freertos_init_state (&ephemeral_key_monitor_task);
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_allocate_static (&ephemeral_key_monitor_task,
		&ephemeral_key_monitor_task_tcb, ephemeral_key_monitor_task_stack,
		EPHEMERAL_KEY_MONITOR_TASK_STACK_WORDS, "Key Mon", CERBERUS_PRIORITY_BACKGROUND);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&ephemeral_key_monitor_task);

	return 0;
}
