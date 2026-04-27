// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SPI_FILTER_CPLD_IRQ_HANDLER_H_
#define SPI_FILTER_CPLD_IRQ_HANDLER_H_

#include <stdbool.h>
#include "spi_filter/spi_filter_cpld.h"
#include "spi_filter/spi_filter_irq_handler.h"


/**
 * Interrupt handler context for the CPLD SPI filter.
 */
struct spi_filter_cpld_irq_handler {
	struct spi_filter_cpld_control *cpld;		/**< The CPLD reporting interrupts. */
	const struct spi_filter_irq_handler *port0;	/**< The IRQ handler for filter port 0. */
	const struct spi_filter_irq_handler *port1;	/**< The IRQ handler for filter port 1. */
	uint8_t blocked[2][32];						/**< Mapping of blocked opcodes detected and logged. */
};


int spi_filter_cpld_irq_handler_init (struct spi_filter_cpld_irq_handler *handler,
	struct spi_filter_cpld_control *cpld, const struct spi_filter_irq_handler *port0,
	const struct spi_filter_irq_handler *port1, bool clear_irq);
void spi_filter_cpld_irq_handler_release (struct spi_filter_cpld_irq_handler *handler);

int spi_filter_cpld_irq_handler_process_irq (struct spi_filter_cpld_irq_handler *handler);


#endif	/* SPI_FILTER_CPLD_IRQ_HANDLER_H_ */
