// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "platform_io_api.h"
#include "telemetry_pcie_handler.h"
#include "common/unused.h"

#define PCIE_LINK_CONTROL_AND_STATUS_REGISTER_ADDR			(0xB0180080)

#define PCIE_COMP_PHY1_LANE_STATUS_REGISTER_ADDR			(0xB0144004)

/**
 * PCIe telemetry handler states
 */
enum {
	TELEMETRY_PCIE_HANDLER_INITIALIZED = 0,		/**< PCIe telemetry handler has been initialized. */
	TELEMETRY_PCIE_HANDLER_RUNNING_NORMAL,		/**< PCIe telemetry handler is running. */
	TELEMETRY_PCIE_HANDLER_RUNNING_ABNORMAL,	/**< PCIe telemetry handler is running, but the values are unexpected */
	TELEMETRY_PCIE_HANDLER_FAULTED,				/**< PCIe telemetry handler has encountered an error. */
};


void telemetry_pcie_handler_prepare (const struct periodic_task_handler *handler)
{
	const struct telemetry_pcie_handler *pcie_handler =
		(const struct telemetry_pcie_handler*) handler;
	int status;

	status = platform_init_timeout (pcie_handler->period, &pcie_handler->state->next);
	if (status == 0) {
		pcie_handler->state->next_valid = true;
	}
	else {
		pcie_handler->state->next_valid = false;
	}
}

const platform_clock* telemetry_pcie_handler_get_next_execution (
	const struct periodic_task_handler *handler)
{
	const struct telemetry_pcie_handler *pcie_handler =
		(const struct telemetry_pcie_handler*) handler;

	if (pcie_handler->state->next_valid) {
		return &pcie_handler->state->next;
	}
	else {
		/* If the next timeout is not valid, just indicate immediate execution. */
		return NULL;
	}
}

/**
 * Check if the current PCIe Link Speed and Width values are as expected.
 *
 * @param pcie_handler Handler for the pcie telemetry interface.
 * @param current_pcie_link_status Pointer to the current_pcie_link_status register
 *
 * @return true if the current PCIe link speed and width are as expected, else false.
 */
static bool telemetry_pcie_handler_link_values_expected (
	const struct telemetry_pcie_handler *pcie_handler,
	LinkControlLinkStatus_t *current_pcie_link_status)
{
	return (current_pcie_link_status->b.PCIE_CAP_LINK_SPEED ==
		pcie_handler->expected_link_status->speed) &&
		   (current_pcie_link_status->b.PCIE_CAP_NEGO_LINK_WIDTH ==
			   pcie_handler->expected_link_status->width);
}

/**
 * Get the current PCIe link values.
 *
 * This function reads the PCIe Link status register after ensuring the PCLK is enabled.
 *
 * @param pcie_handler Handler for the pcie telemetry interface.
 * @param current_pcie_link_status Output pointer to the current_pcie_link_status variable
 *
 * @return 0 if the PCIe link register read is successful, else an error code.
 */
static int telemetry_pcie_handler_get_current_link_status (
	const struct telemetry_pcie_handler *pcie_handler,
	LinkControlLinkStatus_t *current_pcie_link_status)
{
	int status;
	LinkControlLinkStatus_t *reg = NULL;
	Comphy1SoclaneStatus0_t *reg_comp_phy_ptr = NULL;
	Comphy1SoclaneStatus0_t reg_comp_phy;

	status = pcie_handler->dmb->map_soc_address (pcie_handler->dmb,
		PCIE_COMP_PHY1_LANE_STATUS_REGISTER_ADDR, sizeof (Comphy1SoclaneStatus0_t),
		HSP_DMB_ACCESS_READ, (void**) &reg_comp_phy_ptr);
	if (status != 0) {
		return status;
	}

	reg_comp_phy = *reg_comp_phy_ptr;

	pcie_handler->dmb->unmap_soc_address (pcie_handler->dmb, reg_comp_phy_ptr);

	/**
	 * Check if the PCLK is enabled before accessing the PCIe Link register. If this is not yet enabled,
	 * do not proceed with monitoring and return.
	 * Accessing PCIe registers before PCLK is enabled causes the HSP to enter into a boot loop.
	 */
	if (reg_comp_phy.b.PM_TXDCLK_PCLK_EN_LANE == 1) {
		status = pcie_handler->dmb->map_soc_address (pcie_handler->dmb,
			PCIE_LINK_CONTROL_AND_STATUS_REGISTER_ADDR, sizeof (LinkControlLinkStatus_t),
			HSP_DMB_ACCESS_READ, (void**) &reg);
		if (status != 0) {
			return status;
		}

		/* Update the PCIe Link Status and Control register value at once to avoid any changes during
		    the execution of the handler. */
		*current_pcie_link_status = *reg;

		pcie_handler->dmb->unmap_soc_address (pcie_handler->dmb, reg);
	}
	else {
		/* If the PCLK is not set, report the link status as 0. */
		current_pcie_link_status->all = 0;
	}

	return 0;
}

void telemetry_pcie_handler_execute (const struct periodic_task_handler *handler)
{
	const struct telemetry_pcie_handler *pcie_handler =
		(const struct telemetry_pcie_handler*) handler;
	int status;
	LinkControlLinkStatus_t current_pcie_link_status;

	status = telemetry_pcie_handler_get_current_link_status (pcie_handler,
		&current_pcie_link_status);
	if (status != 0) {
		if (pcie_handler->state->current_state != TELEMETRY_PCIE_HANDLER_FAULTED) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_TELEMETRY_PCIE_FAULTED, status, 0);
			pcie_handler->state->current_state = TELEMETRY_PCIE_HANDLER_FAULTED;
		}

		telemetry_pcie_handler_prepare (handler);

		return;
	}

	switch (pcie_handler->state->current_state) {
		case TELEMETRY_PCIE_HANDLER_INITIALIZED:
			if (telemetry_pcie_handler_link_values_expected (pcie_handler,
				&current_pcie_link_status)) {
				pcie_handler->state->current_state =
					TELEMETRY_PCIE_HANDLER_RUNNING_NORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_PCIE_RUNNING,
					current_pcie_link_status.b.PCIE_CAP_LINK_SPEED,
					current_pcie_link_status.b.PCIE_CAP_NEGO_LINK_WIDTH);
			}
			else {
				pcie_handler->state->current_state =
					TELEMETRY_PCIE_HANDLER_RUNNING_ABNORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_PCIE_LINK,
					current_pcie_link_status.b.PCIE_CAP_LINK_SPEED,
					current_pcie_link_status.b.PCIE_CAP_NEGO_LINK_WIDTH);
				pcie_handler->state->previous_pcie_link_status =
					current_pcie_link_status;
			}

			break;

		case TELEMETRY_PCIE_HANDLER_RUNNING_NORMAL:
			if (!telemetry_pcie_handler_link_values_expected (pcie_handler,
				&current_pcie_link_status)) {
				pcie_handler->state->current_state =
					TELEMETRY_PCIE_HANDLER_RUNNING_ABNORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_PCIE_LINK,
					current_pcie_link_status.b.PCIE_CAP_LINK_SPEED,
					current_pcie_link_status.b.PCIE_CAP_NEGO_LINK_WIDTH);
				pcie_handler->state->previous_pcie_link_status =
					current_pcie_link_status;
			}
			break;

		case TELEMETRY_PCIE_HANDLER_RUNNING_ABNORMAL:
			if (telemetry_pcie_handler_link_values_expected (pcie_handler,
				&current_pcie_link_status)) {
				pcie_handler->state->current_state =
					TELEMETRY_PCIE_HANDLER_RUNNING_NORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_PCIE_RUNNING,
					current_pcie_link_status.b.PCIE_CAP_LINK_SPEED,
					current_pcie_link_status.b.PCIE_CAP_NEGO_LINK_WIDTH);
			}
			else {
				if ((pcie_handler->state->previous_pcie_link_status.b.PCIE_CAP_LINK_SPEED !=
					current_pcie_link_status.b.PCIE_CAP_LINK_SPEED) ||
					(pcie_handler->state->previous_pcie_link_status.b.PCIE_CAP_NEGO_LINK_WIDTH !=
					current_pcie_link_status.b.PCIE_CAP_NEGO_LINK_WIDTH)) {
					debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING,
						DEBUG_LOG_COMPONENT_MANTICORE,
						MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_PCIE_LINK,
						current_pcie_link_status.b.PCIE_CAP_LINK_SPEED,
						current_pcie_link_status.b.PCIE_CAP_NEGO_LINK_WIDTH);
					pcie_handler->state->previous_pcie_link_status =
						current_pcie_link_status;
				}
			}
			break;

		case TELEMETRY_PCIE_HANDLER_FAULTED:
			if (telemetry_pcie_handler_link_values_expected (pcie_handler,
				&current_pcie_link_status)) {
				pcie_handler->state->current_state =
					TELEMETRY_PCIE_HANDLER_RUNNING_NORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_PCIE_RUNNING,
					current_pcie_link_status.b.PCIE_CAP_LINK_SPEED,
					current_pcie_link_status.b.PCIE_CAP_NEGO_LINK_WIDTH);
			}
			else {
				pcie_handler->state->current_state =
					TELEMETRY_PCIE_HANDLER_RUNNING_ABNORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_PCIE_LINK,
					current_pcie_link_status.b.PCIE_CAP_LINK_SPEED,
					current_pcie_link_status.b.PCIE_CAP_NEGO_LINK_WIDTH);
				pcie_handler->state->previous_pcie_link_status =
					current_pcie_link_status;
			}
			break;
	}

	telemetry_pcie_handler_prepare (handler);
}

/**
 * Initialize a handler for telemetry PCIe monitor.
 *
 * @param handler The telemetry pcie handler to initialize.
 * @param state Variable context for the handler. This must be uninitialized.
 * @param sensor Temperature Sensor to be used with this handler
 * @param period_ms The amount of time between telemetry temperature monitoring requests, in milliseconds.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int telemetry_pcie_handler_init (struct telemetry_pcie_handler *handler,
	struct telemetry_pcie_handler_state *state, const struct hsp_dmb *dmb,
	const struct telemetry_pcie_link_status *expected_link_status, uint32_t period_ms)
{
	if (handler == NULL) {
		return TELEMETRY_PCIE_HANDLER_INVALID_ARGUMENT;
	}

	handler->base.prepare = telemetry_pcie_handler_prepare;
	handler->base.get_next_execution = telemetry_pcie_handler_get_next_execution;
	handler->base.execute = telemetry_pcie_handler_execute;

	handler->dmb = dmb;
	handler->expected_link_status = expected_link_status;
	handler->state = state;
	handler->period = period_ms;

	return telemetry_pcie_handler_init_state (handler);
}

/**
 * Initialize only the variable state for a telemetry pcie handler.  The rest of the handler
 * is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param handler The telemetry pcie handler that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int telemetry_pcie_handler_init_state (const struct telemetry_pcie_handler *handler)
{
	if ((handler == NULL) || (handler->state == NULL) || (handler->dmb == NULL) ||
		(handler->expected_link_status == NULL)) {
		return TELEMETRY_PCIE_HANDLER_INVALID_ARGUMENT;
	}

	memset (handler->state, 0, sizeof (struct telemetry_pcie_handler_state));

	return 0;
}

/**
 * Release the resources used by a telemetry pcie handler.
 *
 * @param handler The handler to release.
 */
void telemetry_pcie_handler_release (const struct telemetry_pcie_handler *handler)
{
	UNUSED (handler);
}
