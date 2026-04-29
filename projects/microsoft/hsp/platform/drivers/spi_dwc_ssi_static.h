// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SPI_DWC_SSI_STATIC_H_
#define SPI_DWC_SSI_STATIC_H_

#include "spi_dwc_ssi.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization */
bool spi_dwc_ssi_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param);

int spi_dwc_ssi_prepare_transaction_polling (const struct spi_dwc_ssi *spi);
void spi_dwc_ssi_handle_transaction_data_polling (const struct spi_dwc_ssi *spi);
int spi_dwc_ssi_prepare_transaction_interrupt (const struct spi_dwc_ssi *spi);
void spi_dwc_ssi_handle_transaction_data_interrupt (const struct spi_dwc_ssi *spi);


/**
 * Initialize a static instance of a SPI driver that actively spins, polling on status registers to
 * complete SPI transactions.  This does not initialize the driver state or the HW block.  Those
 * will need to be initialized separately with a call to spi_dwc_ssi_init_state.
 *
 * The interrupt handler will be null for instances initialized in this way.
 * The Tx/Rx FIFO bytes will be swapped for instances initialized in this way.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for SPI driver.
 * @param regs_ptr Base address for the SPI HW registers.
 * @param creg_ptr Base address for the SSI slave select logic within the CREG wrapper.
 */
#define	spi_dwc_ssi_static_init_polling(state_ptr, regs_ptr, creg_ptr) { \
		.base = hsp_interrupt_handler_static_init (NULL), \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.creg = creg_ptr, \
		.no_byte_swap = false, \
		.prepare_transaction = spi_dwc_ssi_prepare_transaction_polling, \
		.handle_transaction_data = spi_dwc_ssi_handle_transaction_data_polling \
	}

/**
 * Initialize a static instance of a SPI driver that actively spins, polling on status registers to
 * complete SPI transactions.  This does not initialize the driver state or the HW block.  Those
 * will need to be initialized separately with a call to spi_dwc_ssi_init_state.
 *
 * The interrupt handler will be null for instances initialized in this way.
 * The Tx/Rx FIFO bytes will not be swapped for instances initialized in this way.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for SPI driver.
 * @param regs_ptr Base address for the SPI HW registers.
 * @param creg_ptr Base address for the SSI slave select logic within the CREG wrapper.
 */
#define	spi_dwc_ssi_static_init_polling_no_byte_swap(state_ptr, regs_ptr, creg_ptr) { \
		.base = hsp_interrupt_handler_static_init (NULL), \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.creg = creg_ptr, \
		.no_byte_swap = true, \
		.prepare_transaction = spi_dwc_ssi_prepare_transaction_polling, \
		.handle_transaction_data = spi_dwc_ssi_handle_transaction_data_polling \
	}

/**
 * Initialize a static instance of a SPI driver that uses interrupts to manage SPI transactions.
 * This does not initialize the driver state or the HW block.  Those will need to be initialized
 * separately with a call to spi_dwc_ssi_init_state.
 *
 * The Tx/Rx FIFO bytes will be swapped for instances initialized in this way.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for SPI driver.
 * @param regs_ptr Base address for the SPI HW registers.
 * @param creg_ptr Base address for the SSI slave select logic within the CREG wrapper.
 */
#define	spi_dwc_ssi_static_init_interrupt(state_ptr, regs_ptr, creg_ptr) { \
		.base = hsp_interrupt_handler_static_init (spi_dwc_ssi_handle_interrupt), \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.creg = creg_ptr, \
		.no_byte_swap = false, \
		.prepare_transaction = spi_dwc_ssi_prepare_transaction_interrupt, \
		.handle_transaction_data = spi_dwc_ssi_handle_transaction_data_interrupt \
	}

/**
 * Initialize a static instance of a SPI driver that uses interrupts to manage SPI transactions.
 * This does not initialize the driver state or the HW block.  Those will need to be initialized
 * separately with a call to spi_dwc_ssi_init_state.
 *
 * The Tx/Rx FIFO bytes will not be swapped for instances initialized in this way.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for SPI driver.
 * @param regs_ptr Base address for the SPI HW registers.
 * @param creg_ptr Base address for the SSI slave select logic within the CREG wrapper.
 */
#define	spi_dwc_ssi_static_init_interrupt_no_byte_swap(state_ptr, regs_ptr, creg_ptr) { \
		.base = hsp_interrupt_handler_static_init (spi_dwc_ssi_handle_interrupt), \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.creg = creg_ptr, \
		.no_byte_swap = true, \
		.prepare_transaction = spi_dwc_ssi_prepare_transaction_interrupt, \
		.handle_transaction_data = spi_dwc_ssi_handle_transaction_data_interrupt \
	}


#endif	/* SPI_DWC_SSI_STATIC_H_ */
