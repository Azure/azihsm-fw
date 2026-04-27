// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "spi_filter/spi_filter_cpld_irq_handler.h"
#include "spi_filter/spi_filter_logging.h"


/**
 * Initialize the IRQ handler for a CPLD SPI filter.
 *
 * @param handler The handler to initialize.
 * @param cpld The CPLD that is generating IRQs to handle.
 * @param port0 The handler for filter port 0 interrupts.
 * @param port1 The handler for filter port 1 interrupts.
 * @param clear_irq Flag indicating if the current interrupt status should be cleared.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int spi_filter_cpld_irq_handler_init (struct spi_filter_cpld_irq_handler *handler,
	struct spi_filter_cpld_control *cpld, const struct spi_filter_irq_handler *port0,
	const struct spi_filter_irq_handler *port1, bool clear_irq)
{
	uint8_t mask = 0xff;
	int status;

	if ((handler == NULL) || (cpld == NULL)) {
		return SPI_FILTER_IRQ_INVALID_ARGUMENT;
	}

	if (port0) {
		mask &= ~(CPLD_INT_P0_DIRTY_MASK | CPLD_INT_P0_BLOCK_MASK | CPLD_INT_P0_ADDR_MODE_MASK);
	}
	if (port1) {
		mask &= ~(CPLD_INT_P1_DIRTY_MASK | CPLD_INT_P1_BLOCK_MASK | CPLD_INT_P1_ADDR_MODE_MASK);
	}

	status = spi_filter_cpld_set_int_mask (cpld, mask);
	if (status != 0) {
		return status;
	}

	if (clear_irq) {
		status = spi_filter_cpld_get_int_status (cpld, &mask);
		if (status != 0) {
			return status;
		}
	}

	memset (handler, 0, sizeof (struct spi_filter_cpld_irq_handler));

	handler->cpld = cpld;
	handler->port0 = port0;
	handler->port1 = port1;

	return 0;
}

/**
 * Release the resources used by the CPLD IRQ handler.
 *
 * @param handler The handler to release.
 */
void spi_filter_cpld_irq_handler_release (struct spi_filter_cpld_irq_handler *handler)
{

}

/**
 * Handle an interrupt for a blocked SPI command.
 *
 * @param handler The handler context for processing.
 * @param port The filter port that blocked the command.
 */
static void spi_filter_cpld_irq_handler_blocked_opcode (struct spi_filter_cpld_irq_handler *handler,
	cpld_flash_port port)
{
	uint8_t opcode;
	int status;
	int retry = 4;

	/* Blocked opcode captures are suspended until the opcode register is read.  Retry reading the
	 * register on failure to keep that interrupt source active. */
	do {
		status = spi_filter_cpld_get_blocked_opcode (handler->cpld, port, &opcode);
		if (status == 0) {
			/* Only log each blocked opcode value once per port. */
			if (!(handler->blocked[port][opcode / 8] & (1U << (opcode % 8)))) {
				status = debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO,
					DEBUG_LOG_COMPONENT_SPI_FILTER, SPI_FILTER_LOGGING_BLOCKED_COMMAND, port,
					opcode);

				if (status == 0) {
					handler->blocked[port][opcode / 8] |= (1U << (opcode % 8));
				}

				status = 0;
			}
		}
		else {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_SPI_FILTER,
				SPI_FILTER_LOGGING_READ_BLOCKED_FAIL, port, status);
		}
	} while ((status != 0) && (--retry));
}

/**
 * Handle an interrupt for a switch in the filter address mode.
 *
 * @param handler The handler context for processing.
 * @param port The filter port that switched address mode.
 */
static void spi_filter_cpld_irq_handler_address_mode (struct spi_filter_cpld_irq_handler *handler,
	cpld_flash_port port)
{
	spi_filter_address_mode mode;
	int status;

	status = spi_filter_cpld_get_address_mode (handler->cpld, port, &mode);
	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_SPI_FILTER,
		SPI_FILTER_LOGGING_ADDRESS_MODE, port, (status == 0) ? mode : (uint32_t) status);
}

/**
 * Process an interrupt received from the CPLD.
 *
 * @param handler The handler context for processing.
 *
 * @return 0 if the IRQ was handled successfully or an error code.
 */
int spi_filter_cpld_irq_handler_process_irq (struct spi_filter_cpld_irq_handler *handler)
{
	uint8_t irq = 0;
	int status;

	if (handler == NULL) {
		return SPI_FILTER_IRQ_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_get_int_status (handler->cpld, &irq);
	if (status != 0) {
		return status;
	}

	/* Don't log interrupts that are only for blocked opcodes.  These get logged differently and it
	 * prevents filling the log with messages for blocked opcode IRQs.
	 *
	 * We also don't need to log here for address mode changes, since those always generate a
	 * different log entry. */
	if (irq & ~(CPLD_INT_P0_BLOCK_MASK | CPLD_INT_P1_BLOCK_MASK | CPLD_INT_P0_ADDR_MODE_MASK |
		CPLD_INT_P1_ADDR_MODE_MASK)) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_SPI_FILTER,
			SPI_FILTER_LOGGING_IRQ_STATUS, 0, irq);
	}

	if ((irq & CPLD_INT_P0_DIRTY_MASK) && handler->port0) {
		handler->port0->ro_flash_dirty (handler->port0);
	}

	if ((irq & CPLD_INT_P1_DIRTY_MASK) && handler->port1) {
		handler->port1->ro_flash_dirty (handler->port1);
	}

	if ((irq & CPLD_INT_P0_BLOCK_MASK) && handler->port0) {
		spi_filter_cpld_irq_handler_blocked_opcode (handler, CPLD_FLASH_PORT_0);
	}

	if ((irq & CPLD_INT_P1_BLOCK_MASK) && handler->port1) {
		spi_filter_cpld_irq_handler_blocked_opcode (handler, CPLD_FLASH_PORT_1);
	}

	if ((irq & CPLD_INT_P0_ADDR_MODE_MASK) && handler->port0) {
		spi_filter_cpld_irq_handler_address_mode (handler, CPLD_FLASH_PORT_0);
	}

	if ((irq & CPLD_INT_P1_ADDR_MODE_MASK) && handler->port1) {
		spi_filter_cpld_irq_handler_address_mode (handler, CPLD_FLASH_PORT_1);
	}

	return 0;
}
