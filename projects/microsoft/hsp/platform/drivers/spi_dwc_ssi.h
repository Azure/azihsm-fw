// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SPI_DWC_SSI_H_
#define SPI_DWC_SSI_H_

#include <stdbool.h>
#include <stdint.h>
#include "platform_api.h"
#include "flash/flash_master.h"
#include "trap/hsp_interrupt_handler.h"


struct DWC_ssi_AHB_Slave;	/* Defined in HSP register definition. */
struct Ssi_regs;			/* Defined in HSP register definition. */

/**
 * Descriptor and management for a requested transfer.  A single transfer may be broken down into
 * multiple SPI transactions for the HW to execute.
 */
struct spi_dwc_ssi_xfer {
	const struct flash_xfer *xfer;	/**< The submitted transfer that is being executed. */
	size_t sent_bytes;				/**< Total number of bytes sent in the overall transfer. */
	int clk_divider;				/**< SPI clock being used for the transfer. */
	uint32_t is_dual_quad;			/**< Indication whether the transaction uses multiple data bits. */
	uint32_t ctrlr0;				/**< The value to write to HW register CTRLR0 for the transfer. */
	uint32_t spi_ctrlr0;			/**< The value to write to HW register SPI_CTRLR0 for the transfer. */
	uint32_t txftlr;				/**< Initial value of the TXFTLR HW register. */
	uint8_t addr_bytes;				/**< The number of address bytes to send for the command. */
	uint8_t *data;					/**< Pointer to the data buffer for the current transaction. */
	bool new_xfer;					/**< Flag indicating if the flash command needs to be sent. */
	uint8_t rx_alignment;			/**< Number of unaligned bytes received to align later transactions. */
	size_t xfer_bytes;				/**< Total number of bytes being sent in the current transaction. */
	size_t xfer_frames;				/**< Total number of frames being sent in the current transaction. */
	size_t xfer_start;				/**< Tx FIFO level for the HW to wait on before starting. */
	size_t bytes_done;				/**< Number of bytes completed for the current transaction. */
	int xfer_status;				/**< Result of the overall transfer. */
};

/**
 * Variable context for the SSI SPI driver.
 */
struct spi_dwc_ssi_state {
	platform_mutex lock;				/**< Lock for synchronization. */
	uint32_t ssi_clock;					/**< Frequency of the internal clock. */
	struct spi_dwc_ssi_xfer *active;	/**< The transaction being executed by the SPI driver. */
	platform_semaphore done;			/**< Semaphore indicating when a transaction has completed. */
};

/**
 * Driver instance for executing SPI transactions using the DesignWare Cores Synchronized Serial
 * Interface HW block from Synopsis.
 */
struct spi_dwc_ssi {
	struct hsp_interrupt_handler base;	/**< Base interface for handling SPI interrupts. */
	struct spi_dwc_ssi_state *state;	/**< Variable context for the driver instance. */
	struct DWC_ssi_AHB_Slave *regs;		/**< Register interface for the SPI HW. */
	struct Ssi_regs *creg;				/**< CREG register interface for SPI slave select logic. */
	bool no_byte_swap;					/**< Byte swap configurability for Tx/Rx FIFO. */

	/**
	 * Execution any per-transaction configuration or initialization before the hardware is
	 * configured for execution.
	 *
	 * @param spi The SPI master executing the transaction.
	 *
	 * @return 0 if the transaction is ready to run or an error code.
	 */
	int (*prepare_transaction) (const struct spi_dwc_ssi *spi);

	/**
	 * Internal handler for sending or receiving data as part of a single SPI transaction.  This
	 * call will block until all data has been queued for sending (for Tx transactions), received
	 * into the destination memory (for Rx transactions), or there is an error detected.
	 *
	 * @param spi The SPI master executing the transaction.
	 */
	void (*handle_transaction_data) (const struct spi_dwc_ssi *spi);
};


int spi_dwc_ssi_init_polling (struct spi_dwc_ssi *spi, struct spi_dwc_ssi_state *state,
	struct DWC_ssi_AHB_Slave *regs, struct Ssi_regs *creg, uint32_t ssi_clock_hz,
	uint32_t spi_clock_hz);
int spi_dwc_ssi_init_polling_no_byte_swap (struct spi_dwc_ssi *spi, struct spi_dwc_ssi_state *state,
	struct DWC_ssi_AHB_Slave *regs, struct Ssi_regs *creg, uint32_t ssi_clock_hz,
	uint32_t spi_clock_hz);
int spi_dwc_ssi_init_interrupt (struct spi_dwc_ssi *spi, struct spi_dwc_ssi_state *state,
	struct DWC_ssi_AHB_Slave *regs, struct Ssi_regs *creg, uint32_t ssi_clock_hz,
	uint32_t spi_clock_hz);
int spi_dwc_ssi_init_interrupt_no_byte_swap (struct spi_dwc_ssi *spi,
	struct spi_dwc_ssi_state *state, struct DWC_ssi_AHB_Slave *regs, struct Ssi_regs *creg,
	uint32_t ssi_clock_hz, uint32_t spi_clock_hz);
int spi_dwc_ssi_init_state (const struct spi_dwc_ssi *spi, uint32_t ssi_clock_hz,
	uint32_t spi_clock_hz);
void spi_dwc_ssi_release (const struct spi_dwc_ssi *spi);

int spi_dwc_ssi_xfer (const struct spi_dwc_ssi *spi, const struct flash_xfer *xfer,
	uint8_t slave_select, uint32_t spi_clock_hz);

int spi_dwc_ssi_get_spi_frequency (const struct spi_dwc_ssi *spi);
int spi_dwc_ssi_set_spi_frequency (const struct spi_dwc_ssi *spi, uint32_t spi_clock_hz);
int spi_dwc_ssi_get_supported_frequency (const struct spi_dwc_ssi *spi, uint32_t spi_clock_hz);

int spi_dwc_ssi_get_rx_sample_delay (const struct spi_dwc_ssi *spi);
int spi_dwc_ssi_set_rx_sample_delay (const struct spi_dwc_ssi *spi, uint32_t rx_sample_delay);

/* This is really just an extension of the flash_master implementation, so FLASH_MASTER_* error
 * codes are used by this driver. */


#endif	/* SPI_DWC_SSI_H_ */
