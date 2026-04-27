// Copyright (c) Microsoft Corporation. All rights reserved.

#include "manticore_sticky_regs.h"
#include "reset_counter_init.h"
#include "cmd_interface/cerberus_protocol_required_commands.h"


/**
 * Manager for tracking device resets.
 *
 * TODO:  Create a static initializer for this type.
 */
struct counter_manager_registers reset_count;


/**
 * Initialize reset counter manager and increment the device reset count.
 *
 * @return 0 if initialized successfully or an error code.
 */
int initialize_and_increment_reset_counter ()
{
	int status;

	status = counter_manager_registers_init (&reset_count,
		MANTICORE_STICKY_REG (MANTICORE_SPRT_RESET_COUNTER),
		MANTICORE_STICKY_REG (MANTICORE_HOST_RESET_COUNTER));
	if (status != 0) {
		return status;
	}

	return counter_manager_registers_increment (&reset_count, CERBERUS_PROTOCOL_CERBERUS_RESET, 0);
}

/**
 * Initialize the reset counter manager using current reset count values.
 *
 * @return 0 if initialized successfully or an error code.
 */
int initialize_reset_counters ()
{
	return counter_manager_registers_init (&reset_count,
		MANTICORE_STICKY_REG (MANTICORE_SPRT_RESET_COUNTER),
		MANTICORE_STICKY_REG (MANTICORE_HOST_RESET_COUNTER));
}
