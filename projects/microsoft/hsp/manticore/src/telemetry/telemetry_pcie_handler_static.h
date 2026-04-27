// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TELEMETRY_PCIE_HANDLER_STATIC_H_
#define TELEMETRY_PCIE_HANDLER_STATIC_H_

#include "telemetry_pcie_handler.h"


/* Internal functions declared to allow for static initialization. */
void telemetry_pcie_handler_prepare (const struct periodic_task_handler *handler);
const platform_clock* telemetry_pcie_handler_get_next_execution (
	const struct periodic_task_handler *handler);
void telemetry_pcie_handler_execute (const struct periodic_task_handler *handler);


/**
 * Constant initializer for the telemetry PCIe handler API.
 */
#define	TELEMETRY_PCIE_HANDLER_API_INIT  { \
		.prepare = telemetry_pcie_handler_prepare, \
		.get_next_execution = telemetry_pcie_handler_get_next_execution, \
		.execute = telemetry_pcie_handler_execute, \
	}


/**
 * Initialize a static instance of a telemetry PCIe handler.  This does not initialize the
 * handler state. This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param dmb The DMB memory object.
 * @param expected_link_status The expected values for the PCIe Link Speed and Width
 * @param state Variable context for the handler.
 * @param period_ms The amount of time between telemetry temperature monitoring requests, in milliseconds.
 */
#define	telemetry_pcie_handler_static_init(state_ptr, dmb_ptr, expected_link_ptr, period_ms) { \
	.base = TELEMETRY_PCIE_HANDLER_API_INIT, \
	.state = state_ptr, \
	.dmb = dmb_ptr, \
	.expected_link_status = expected_link_ptr, \
	.period = period_ms, \
}


#endif	/* TELEMETRY_PCIE_HANDLER_STATIC_H_ */
