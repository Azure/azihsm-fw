// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SPI_FILTER_HSP_IRQ_HANDLER_H_
#define SPI_FILTER_HSP_IRQ_HANDLER_H_

#include <stdbool.h>
#include "platform_api.h"
#include "spi_filter/spi_filter_hsp.h"
#include "spi_filter/spi_filter_irq_handler.h"
#include "system/periodic_task.h"
#include "trap/hsp_interrupt_handler.h"


/**
 * Variable context for the HSP SPI filter interrupt handler.
 */
struct spi_filter_hsp_irq_handler_state {
	platform_semaphore filter_event;	/**< Signal that an interrupt event occurred. */
	volatile uint32_t event_irq;		/**< Filter interrupt status for the event. */
	uint8_t blocked[256 / 8];			/**< Mapping of blocked opcodes detected and logged. */
};

/**
 * Interrupt handler for the HSP SPI filter.
 */
struct spi_filter_hsp_irq_handler {
	struct periodic_task_handler base;				/**< Task handler for processing interrupt events. */
	struct hsp_interrupt_handler base_irq;			/**< IRQ handler for processing hardware events. */
	struct spi_filter_hsp_irq_handler_state *state;	/**< Variable context for the IRQ handler. */
	const struct spi_filter_hsp *filter;			/**< The SPI filter generating the interrupts. */
	const struct spi_filter_irq_handler *port_irq;	/**< Host port processing for SPI filter interrupts. */
};


int spi_filter_hsp_irq_handler_init (struct spi_filter_hsp_irq_handler *handler,
	struct spi_filter_hsp_irq_handler_state *state, const struct spi_filter_hsp *filter,
	const struct spi_filter_irq_handler *port_irq, bool clear_irq);
int spi_filter_hsp_irq_handler_init_state (const struct spi_filter_hsp_irq_handler *handler,
	bool clear_irq);
void spi_filter_hsp_irq_handler_release (const struct spi_filter_hsp_irq_handler *handler);


#endif	/* SPI_FILTER_HSP_IRQ_HANDLER_H_ */
