// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SPI_FILTER_HSP_IRQ_HANDLER_STATIC_H_
#define SPI_FILTER_HSP_IRQ_HANDLER_STATIC_H_

#include "spi_filter/spi_filter_hsp_irq_handler.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */
void spi_filter_hsp_irq_handler_prepare (const struct periodic_task_handler *handler);
const platform_clock* spi_filter_hsp_irq_handler_get_next_execution (
	const struct periodic_task_handler *handler);
void spi_filter_hsp_irq_handler_execute (const struct periodic_task_handler *handler);

bool spi_filter_hsp_irq_handler_handle_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param);


/**
 * Constant initializer for the task API.
 */
#define	SPI_FILTER_HSP_IRQ_HANDLER_API_INIT  { \
		.prepare = spi_filter_hsp_irq_handler_prepare, \
		.get_next_execution = spi_filter_hsp_irq_handler_get_next_execution, \
		.execute = spi_filter_hsp_irq_handler_execute \
	}


/**
 * Initialize a static handler for HSP SPI filter interrupts.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the interrupt handler.
 * @param filter_ptr The HSP SPI filter generating the interrupts.
 * @param port_irq_ptr Port handler for SPI filter events.
 */
#define	spi_filter_hsp_irq_handler_static_init(state_ptr, filter_ptr, port_irq_ptr)	{ \
		.base = SPI_FILTER_HSP_IRQ_HANDLER_API_INIT, \
		.base_irq = \
			hsp_interrupt_handler_static_init (spi_filter_hsp_irq_handler_handle_interrupt), \
		.state = state_ptr, \
		.filter = filter_ptr, \
		.port_irq = port_irq_ptr, \
	}


#endif	/* SPI_FILTER_HSP_IRQ_HANDLER_STATIC_H_ */
