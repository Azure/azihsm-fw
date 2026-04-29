// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "spi_dwc_ssi.h"
#include "common/unused.h"
#include "flash/flash_common.h"


/**
 * Configuration to put the SPI HW into master mode.
 */
#define	SPI_DWC_SSI_MASTER      \
	DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_SSI_IS_MST_SET ( \
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_SSI_IS_MST_SSI_IS_MST_MASTER)

/**
 * Defines the command code size to be 8 bits.
 */
#define	SPI_DWC_SSI_8BIT_INSTRUCTION \
	DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_INST_L_SET ( \
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_INST_L_INST_L_INST_L8)

/**
 * Configuration to enable clock stretching, preventing Rx/Tx FIFO over/underflows.
 */
#define	SPI_DWC_SSI_ENABLE_CLOCK_STRETCHING \
	DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_CLK_STRETCH_EN_FIELD_MASK

/**
 * Configuration values to specify tha number of address bytes in a command.
 */
enum {
	/**< The transaction does not contain an address. */
	SPI_DWC_SSI_NO_ADDRESS =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_ADDR_L_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_ADDR_L_ADDR_L_ADDR_L0),
	/**< The transaction contains 3 bytes of address. */
	SPI_DWC_SSI_3BYTE_ADDRESS =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_ADDR_L_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_ADDR_L_ADDR_L_ADDR_L24),
	/**< The transaction contains 4 bytes of address. */
	SPI_DWC_SSI_4BYTE_ADDRESS =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_ADDR_L_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_ADDR_L_ADDR_L_ADDR_L32),
};

/**
 * Configuration values for the SPI transaction mode.
 */
enum {
	/** Transaction uses Single SPI mode. */
	SPI_DWC_SSI_SINGLE =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_SPI_FRF_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_SPI_FRF_SPI_FRF_SPI_STANDARD),
	/** Transaction uses Dual SPI mode. */
	SPI_DWC_SSI_DUAL =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_SPI_FRF_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_SPI_FRF_SPI_FRF_SPI_DUAL),
	/** Transaction uses Quad SPI mode. */
	SPI_DWC_SSI_QUAD =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_SPI_FRF_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_SPI_FRF_SPI_FRF_SPI_QUAD),
};

/**
 * Configuration values to control how each phase of the transaction is sent.
 */
enum {
	/** The command and address phase are sent in single SPI mode. */
	SPI_DWC_SSI_1_1_X =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_TRANS_TYPE_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_TRANS_TYPE_TRANS_TYPE_TT0),
	/** Only the command phase is sent in single SPI mode. */
	SPI_DWC_SSI_1_X_X =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_TRANS_TYPE_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_TRANS_TYPE_TRANS_TYPE_TT1),
	/** Nothing is sent in single SPI mode. */
	SPI_DWC_SSI_X_X_X =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_TRANS_TYPE_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_TRANS_TYPE_TRANS_TYPE_TT2),
};

/**
 * Configuration values to control the frame size of the transaction.
 */
enum {
	/** Use an 8 bit frame.  Only bits [0:7] of the Tx/Rx FIFOs will have valid data. */
	SPI_DWC_SSI_8BIT_FRAME =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_DFS_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_DFS_DFS_DFS_08_BIT),
	/** Use a 32 bit frame.  All bits in the Tx/Rx FIFOs will be valid. */
	SPI_DWC_SSI_32BIT_FRAME =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_DFS_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_DFS_DFS_DFS_32_BIT),
};

/**
 * Configuration values to control the transfer direction.
 */
enum {
	/** The transfer is sending data to the flash device. */
	SPI_DWC_SSI_TX =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_TMOD_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_TMOD_TMOD_TX_ONLY),
	/** The transfer is reading data from the flash device using standard, single-bit SPI. */
	SPI_DWC_SSI_SINGLE_RX =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_TMOD_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_TMOD_TMOD_EEPROM_READ),
	/** The transfer is reading data from the flash device using a multi-bit transfer mode. */
	SPI_DWC_SSI_MULTI_RX =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_TMOD_SET (
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_TMOD_TMOD_RX_ONLY),
};

/**
 * Calculate wait cycles for a single SPI transaction.
 */
#define	SPI_DWC_SSI_SINGLE_WAIT_CYCLES(x)   \
	DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_WAIT_CYCLES_SET ((x) * 8)

/**
 * Calculate wait cycles for a dual SPI transaction.
 */
#define	SPI_DWC_SSI_DUAL_WAIT_CYCLES(x) \
	DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_WAIT_CYCLES_SET ((x) * 4)

/**
 * Calculate wait cycles for a quad SPI transaction.
 */
#define	SPI_DWC_SSI_QUAD_WAIT_CYCLES(x) \
	DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_WAIT_CYCLES_SET ((x) * 2)

/**
 * Set the SPI_CTRLR0 value to indicate no command, address, or dummy bytes.  All these values need
 * to be 0, so simply clear the relevant bits.
 *
 * DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_INST_L_INST_L_INST_L0 == 0
 * DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_ADDR_L_ADDR_L_ADDR_L0 == 0
 */
#define	DWC_SPI_SSI_XFER_DATA_ONLY(x)   \
	((x) & ~(DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_INST_L_FIELD_MASK | \
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_ADDR_L_FIELD_MASK | \
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_WAIT_CYCLES_FIELD_MASK))

/**
 * Determine if an address is word aligned or not.
 */
#define	SPI_DWC_SSI_IS_UINT32_ALIGNED(x)		(((uintptr_t) (x) & 0x03) == 0)

/**
 * Helper macro to get the bit mask for SPI register values.
 *
 * @param reg The register name.
 * @param bit The value name in the register.
 */
#define	SPI_DWC_SSI_REG_BIT_MASK(reg, bit)  \
	DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_ ## reg ## _ ## bit ## _FIELD_MASK

/**
 * Maximum number of entries that can be held in the Tx FIFO.
 */
#define	SPI_DWC_SSI_TX_FIFO_DEPTH	(1U << DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_TXFTLR_TXFTHR_WIDTH)

/**
 * Maximum number of entries that can be held in the Rx FIFO.
 */
#define	SPI_DWC_SSI_RX_FIFO_DEPTH	(1U << DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_RXFTLR_RFT_WIDTH)

/**
 * Maximum number of frames that can be sent in a single hardware transaction.  This is not the same
 * as the number of bytes that can be transfers, since Dual/Quad transfers use 4-byte frames.
 */
#define	SPI_DWC_SSI_MAX_FRAMES		(64 * 1024)

/**
 * Specify the chip select to use for a SPI transfer.
 *
 * @param x The CS number.  0 for the first CS, 1 for the second, etc.
 */
#define	SPI_DWC_SSI_CS(x)			(1U << (x))

/**
 * Assert the chip select to use for a SPI transfer using the CREG override of the SPI hardware CS
 * output.
 *
 * @param x The CS number.  0 for the first CS, 1 for the second, etc.
 */
#define	SPI_DWC_SSI_CREG_CS_ASSERT(x)   \
	((SSI_REGS_SPI_SLAVE_SELECT_S0_WR_FIELD_MASK | SSI_REGS_SPI_SLAVE_SELECT_S0_SEL_FIELD_MASK) << \
		((x) * 3))

/**
 * Deassert the chip select being used for a SPI transfer using the CREG override of the SPI
 * hardware CS output.
 *
 * @param x The CS number.  0 for the first CS, 1 for the second, etc.
 */
#define	SPI_DWC_SSI_CREG_CS_DEASSERT(x) \
	((SSI_REGS_SPI_SLAVE_SELECT_S0_WR_FIELD_MASK | SSI_REGS_SPI_SLAVE_SELECT_S0_SEL_FIELD_MASK | \
		SSI_REGS_SPI_SLAVE_SELECT_S0_SS_FIELD_MASK) << ((x) * 3))


/**
 * Calculate the clock divider to use with the SPI HW to achieve at most the specified frequency for
 * the SPI bus clock.  The output clock will not exceed the specified target.
 *
 * @param ssi_clock Frequency of the internal clock for the SPI HW, in Hz.
 * @param spi_clock Target frequency for the external SPI clock, in Hz.
 *
 * @return The divider value that should be written to the baud rate register or an error code.
 */
static int spi_dwc_ssi_calculate_clock_divider (uint32_t ssi_clock, uint32_t spi_clock)
{
	int div = ssi_clock / spi_clock;

	/* Keep incrementing the divider until the SPI clock that will be generated is no higher than
	 * the requested frequency.  This will happen if the target frequency is not an exact multiple
	 * of the main clock. */
	while ((ssi_clock / div) > spi_clock) {
		div++;
	}

	if (div > 0xfffe) {
		return FLASH_MASTER_FREQ_OUT_OF_RANGE;
	}

	return (div & 0x1) ? (div + 1) : div;
}

/**
 * Read a register to clear a SPI interrupt.
 *
 * @param clr_reg The register that should be read.
 *
 * @return The value of the register.
 */
static uint32_t spi_dwc_ssi_clear_interrupt (volatile uint32_t *clr_reg)
{
	return *clr_reg;
}

/**
 * Fill the Tx FIFO with data for transmission until there is no more space in the FIFO or no more
 * data to send.
 *
 * @param spi The SPI master transmitting data.
 */
static void spi_dwc_ssi_fill_tx_fifo (const struct spi_dwc_ssi *spi)
{
	struct spi_dwc_ssi_xfer *spi_xfer = spi->state->active;
	uint32_t tx;

	while ((spi->regs->ssic_address_block.SR & SPI_DWC_SSI_REG_BIT_MASK (SR, TFNF)) &&
		(spi_xfer->bytes_done < spi_xfer->xfer_bytes)) {
		if (spi_xfer->is_dual_quad) {
			/* Dual/Quad commands need to put 4 bytes in each FIFO entry.  We already know the
			 * length is 4-byte aligned, so we don't need to be concerned with overrunning the data
			 * buffer.  However, we still need to handle cases where the input buffer is not word
			 * aligned. */
			if (!SPI_DWC_SSI_IS_UINT32_ALIGNED (&spi_xfer->data[spi_xfer->bytes_done])) {
				memcpy (&tx, &spi_xfer->data[spi_xfer->bytes_done], sizeof (tx));
			}
			else {
				tx = *((uint32_t*) &spi_xfer->data[spi_xfer->bytes_done]);
			}

			/* Swap FIFO byte ordering if necessary
			 * SPI Flash to send data MSB -> LSB
			 * SPI Bridge (no byte swap) LSB -> MSB */
			if (!spi->no_byte_swap) {
				tx = common_math_swap_bytes_uint32 (tx);
			}

			spi->regs->ssic_address_block.DR0 = tx;
			spi_xfer->bytes_done += 4;
		}
		else {
			/* Single SPI commands only put one data byte per FIFO entry. */
			spi->regs->ssic_address_block.DR0 = spi_xfer->data[spi_xfer->bytes_done++];
		}
	}
}

/**
 * Read from the Rx FIFO until the FIFO is empty or there is no more buffer space for data.
 *
 * @param spi The SPI master receiving data.
 */
static void spi_dwc_ssi_drain_rx_fifo (const struct spi_dwc_ssi *spi)
{
	struct spi_dwc_ssi_xfer *spi_xfer = spi->state->active;
	uint32_t rx;

	while ((spi->regs->ssic_address_block.SR & SPI_DWC_SSI_REG_BIT_MASK (SR, RFNE)) &&
		(spi_xfer->bytes_done < spi_xfer->xfer_bytes)) {
		rx = spi->regs->ssic_address_block.DR0;

		if (spi_xfer->is_dual_quad) {
			/* Dual/Quad commands have 4 bytes packed into a single FIFO entry.
			 * Swap the FIFO byte order if necessary (i.e. SPI Flash). */
			if (!spi->no_byte_swap) {
				rx = common_math_swap_bytes_uint32 (rx);
			}

			if (SPI_DWC_SSI_IS_UINT32_ALIGNED (&spi_xfer->data[spi_xfer->bytes_done]) &&
				(spi_xfer->xfer_bytes >= 4) &&
				(spi_xfer->bytes_done <= (spi_xfer->xfer_bytes - 4))) {
				/* The data buffer is 4-byte aligned and has enough space for the entire word of
				 * data.  Copy it in one memory access. */
				*((uint32_t*) &spi_xfer->data[spi_xfer->bytes_done]) = rx;
				spi_xfer->bytes_done += 4;
			}
			else {
				/* This covers two scenarios requiring special processing:
				 * 1. When this is the last block of data being received, but there is not enough
				 *    space in the buffer for the entire 4 bytes of data.
				 * 2. When the output buffer is not 4-byte aligned.  Attempting word accesses for
				 *    misaligned addresses causes a RISC-V trap, but byte access for these addresses
				 *    is allowed. */
				uint8_t bytes = 4;

				while ((spi_xfer->bytes_done != spi_xfer->xfer_bytes) && bytes) {
					spi_xfer->data[spi_xfer->bytes_done++] = (uint8_t) rx;
					rx >>= 8;
					bytes--;
				}
			}
		}
		else {
			/* Single SPI commands only have 8 valid bits per FIFO entry. */
			spi_xfer->data[spi_xfer->bytes_done++] = rx;
		}
	}
}

bool spi_dwc_ssi_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param)
{
	const struct spi_dwc_ssi *spi = (const struct spi_dwc_ssi*) handler;
	struct spi_dwc_ssi_xfer *spi_xfer;
	uint32_t int_status;

	UNUSED (param);

	if (spi == NULL) {
		/* No SPI master. */
		return false;
	}

	spi_xfer = spi->state->active;
	if (spi_xfer == NULL) {
		/* No active transaction on the SPI master. */
		return false;
	}

	int_status = spi->regs->ssic_address_block.ISR;
	if (int_status == 0) {
		/* The interrupt is not for this instance. */
		return false;
	}

	if (int_status & SPI_DWC_SSI_REG_BIT_MASK (ISR, TXUIS)) {
		/* If the Tx FIFO underflows, the transaction is not correct and should be aborted.
		 * However, it seems this interrupt never gets triggered.  If the Tx FIFO actually empties
		 * all the way, the transaction terminates prematurely without any obvious indication of
		 * this scenario.  Because of this behavior, it is critical that the Tx FIFO never be
		 * allowed to fully empty until all data has been sent.
		 *
		 * A different configuration of the SPI hardware would prevent this scenario from happening
		 * and pause the transaction until data is available in the FIFO.  Future HSP versions may
		 * have this configuration and not be susceptible to this negative behavior. */
		spi_xfer->xfer_status = FLASH_MASTER_TX_FIFO_UNDERFLOW;
	}
	else if (int_status & SPI_DWC_SSI_REG_BIT_MASK (ISR, TXEIS)) {
		spi_dwc_ssi_fill_tx_fifo (spi);
	}

	if (int_status & SPI_DWC_SSI_REG_BIT_MASK (ISR, RXOIS)) {
		/* If the Rx FIFO has overflowed, data has been lost.  Abort the transaction and mark it as
		 * bad. */
		spi_xfer->xfer_status = FLASH_MASTER_RX_FIFO_OVERFLOW;
	}
	else if (int_status & SPI_DWC_SSI_REG_BIT_MASK (ISR, RXFIS)) {
		spi_dwc_ssi_drain_rx_fifo (spi);
	}

	if ((spi_xfer->bytes_done >= spi_xfer->xfer_bytes) || (spi_xfer->xfer_status != 0)) {
		/* No more processing is necessary for the transaction.  Disable any further interrupts. */
		platform_semaphore_post_from_isr (&spi->state->done);
		spi->regs->ssic_address_block.IMR = 0;
	}

	return true;
}

int spi_dwc_ssi_prepare_transaction_interrupt (const struct spi_dwc_ssi *spi)
{
	return platform_semaphore_reset (&spi->state->done);
}

void spi_dwc_ssi_handle_transaction_data_interrupt (const struct spi_dwc_ssi *spi)
{
	struct spi_dwc_ssi_xfer *spi_xfer = spi->state->active;
	int status;

	spi_xfer->bytes_done = 0;

	/* Enable interrupts, which will take care of filling and draining FIFOs for the transfer. */
	if (flash_xfer_is_tx (spi_xfer->xfer)) {
		/* Make sure the Tx FIFO has as much data as possible right away. */
		spi_dwc_ssi_fill_tx_fifo (spi);

		spi->regs->ssic_address_block.IMR = SPI_DWC_SSI_REG_BIT_MASK (IMR, TXEIM) |
			SPI_DWC_SSI_REG_BIT_MASK (IMR, TXUIM);
	}
	else {
		spi->regs->ssic_address_block.IMR = SPI_DWC_SSI_REG_BIT_MASK (IMR, RXFIM) |
			SPI_DWC_SSI_REG_BIT_MASK (IMR, RXOIM);
	}

	status = platform_semaphore_wait (&spi->state->done, 0);
	if (status != 0) {
		spi_xfer->xfer_status = status;
	}
}

int spi_dwc_ssi_prepare_transaction_polling (const struct spi_dwc_ssi *spi)
{
	UNUSED (spi);

	/* When using polling, it is necessary that the task not get swapped out in the middle of a
	 * transfer so that Tx and Rx FIFOs can be processed correctly.  Otherwise, the transfer will
	 * get corrupted due to FIFO over/underflow.
	 *
	 * Ignore the status here.  If the transfer fails because the OS is still running, the SPI
	 * transfer error will be reported. */
	platform_os_suspend_scheduler ();

	return 0;
}

void spi_dwc_ssi_handle_transaction_data_polling (const struct spi_dwc_ssi *spi)
{
	struct spi_dwc_ssi_xfer *spi_xfer = spi->state->active;
	uint32_t tx_underflow;
	uint32_t rx_overflow;

	/* Set the chip select to start the transfer. */
	spi_xfer->bytes_done = 0;

	while (spi_xfer->bytes_done < spi_xfer->xfer_bytes) {
		if (flash_xfer_is_tx (spi_xfer->xfer)) {
			/* Check for a error condition with the Tx FIFO.  If this happens, then the transfer is
			 * corrupt, so abort it.  This may mean the flash is in an intermediate state that upper
			 * layers would need to deal with. */
			tx_underflow = spi->regs->ssic_address_block.TXEICR;
			if (tx_underflow) {
				spi_xfer->xfer_status = FLASH_MASTER_TX_FIFO_UNDERFLOW;
				goto exit;
			}

			spi_dwc_ssi_fill_tx_fifo (spi);
		}
		else {
			/* Check to make sure we haven't lost any data.  If any received data was dropped, abort
			 * the transfer and return an error. */
			rx_overflow = spi->regs->ssic_address_block.RXOICR;
			if (rx_overflow) {
				spi_xfer->xfer_status = FLASH_MASTER_RX_FIFO_OVERFLOW;
				goto exit;
			}

			spi_dwc_ssi_drain_rx_fifo (spi);
		}
	}

exit:
	/* Resume the OS scheduler now that FIFO management is done. */
	platform_os_resume_scheduler ();
}

/**
 * Initialize a DesignWare SPI instance that actively spins, polling on status registers to complete
 * SPI transactions.
 *
 * The interrupt handler will be null for instances initialized in this way.
 * The Tx/Rx FIFO bytes will be swapped for instances initialized in this way.
 *
 * @param spi The SPI driver instance to initialize.
 * @param state Variable context for the SPI driver instance.  This must be uninitialized.
 * @param regs Base address for the SPI HW registers.
 * @param creg Base address for the SSI slave select logic within the CREG wrapper.
 * @param ssi_clock_hz Frequency of the internal clock for the SPI HW, in Hz.
 * @param spi_clock_hz Desired frequency for the external SPI bus clock, in Hz.  This must be at
 * most half the rate of the internal clock.  The resulting value will be the closest frequency that
 * can be achieved without exceeding the specified SPI clock frequency.
 *
 * @return 0 if the driver was initialized successfully or an error code.
 */
int spi_dwc_ssi_init_polling (struct spi_dwc_ssi *spi, struct spi_dwc_ssi_state *state,
	struct DWC_ssi_AHB_Slave *regs, struct Ssi_regs *creg, uint32_t ssi_clock_hz,
	uint32_t spi_clock_hz)
{
	if ((spi == NULL) || (state == NULL) || (regs == NULL) || (creg == NULL)) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	memset (spi, 0, sizeof (struct spi_dwc_ssi));

	spi->state = state;
	spi->regs = regs;
	spi->creg = creg;
	spi->no_byte_swap = false;

	spi->prepare_transaction = spi_dwc_ssi_prepare_transaction_polling;
	spi->handle_transaction_data = spi_dwc_ssi_handle_transaction_data_polling;

	return spi_dwc_ssi_init_state (spi, ssi_clock_hz, spi_clock_hz);
}

/**
 * Initialize a DesignWare SPI instance that actively spins, polling on status registers to complete
 * SPI transactions.
 *
 * The interrupt handler will be null for instances initialized in this way.
 * The Tx/Rx FIFO bytes will not be swapped for instances initialized in this way.
 *
 * @param spi The SPI driver instance to initialize.
 * @param state Variable context for the SPI driver instance.  This must be uninitialized.
 * @param regs Base address for the SPI HW registers.
 * @param creg Base address for the SSI slave select logic within the CREG wrapper.
 * @param ssi_clock_hz Frequency of the internal clock for the SPI HW, in Hz.
 * @param spi_clock_hz Desired frequency for the external SPI bus clock, in Hz.  This must be at
 * most half the rate of the internal clock.  The resulting value will be the closest frequency that
 * can be achieved without exceeding the specified SPI clock frequency.
 *
 * @return 0 if the driver was initialized successfully or an error code.
 */
int spi_dwc_ssi_init_polling_no_byte_swap (struct spi_dwc_ssi *spi, struct spi_dwc_ssi_state *state,
	struct DWC_ssi_AHB_Slave *regs, struct Ssi_regs *creg, uint32_t ssi_clock_hz,
	uint32_t spi_clock_hz)
{
	int status;

	status = spi_dwc_ssi_init_polling (spi, state, regs, creg, ssi_clock_hz, spi_clock_hz);
	if (status != 0) {
		return status;
	}

	spi->no_byte_swap = true;

	return status;
}

/**
 * Initialize a DesignWare SPI instance that uses interrupts to manage SPI transactions.
 *
 * The interrupt handler will be populated for instances initialized in this way.
 * The Tx/Rx FIFO bytes will be swapped for instances initialized in this way.
 *
 * @param spi The SPI driver instance to initialize.
 * @param state Variable context for the SPI driver instance.  This must be uninitialized.
 * @param regs Base address for the SPI HW registers.
 * @param creg Base address for the SSI slave select logic within the CREG wrapper.
 * @param ssi_clock_hz Frequency of the internal clock for the SPI HW, in Hz.
 * @param spi_clock_hz Desired frequency for the external SPI bus clock, in Hz.  This must be at
 * most half the rate of the internal clock.  The resulting value will be the closest frequency that
 * can be achieved without exceeding the specified SPI clock frequency.
 *
 * @return 0 if the driver was initialized successfully or an error code.
 */
int spi_dwc_ssi_init_interrupt (struct spi_dwc_ssi *spi, struct spi_dwc_ssi_state *state,
	struct DWC_ssi_AHB_Slave *regs, struct Ssi_regs *creg, uint32_t ssi_clock_hz,
	uint32_t spi_clock_hz)
{
	if ((spi == NULL) || (state == NULL) || (regs == NULL) || (creg == NULL)) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	memset (spi, 0, sizeof (struct spi_dwc_ssi));

	spi->base.handle_interrupt = spi_dwc_ssi_handle_interrupt;

	spi->state = state;
	spi->regs = regs;
	spi->creg = creg;
	spi->no_byte_swap = false;

	spi->prepare_transaction = spi_dwc_ssi_prepare_transaction_interrupt;
	spi->handle_transaction_data = spi_dwc_ssi_handle_transaction_data_interrupt;

	return spi_dwc_ssi_init_state (spi, ssi_clock_hz, spi_clock_hz);
}

/**
 * Initialize a DesignWare SPI instance that uses interrupts to manage SPI transactions.
 *
 * The interrupt handler will be populated for instances initialized in this way.
 * The Tx/Rx FIFO bytes will not be swapped for instances initialized in this way.
 *
 * @param spi The SPI driver instance to initialize.
 * @param state Variable context for the SPI driver instance.  This must be uninitialized.
 * @param regs Base address for the SPI HW registers.
 * @param creg Base address for the SSI slave select logic within the CREG wrapper.
 * @param ssi_clock_hz Frequency of the internal clock for the SPI HW, in Hz.
 * @param spi_clock_hz Desired frequency for the external SPI bus clock, in Hz.  This must be at
 * most half the rate of the internal clock.  The resulting value will be the closest frequency that
 * can be achieved without exceeding the specified SPI clock frequency.
 *
 * @return 0 if the driver was initialized successfully or an error code.
 */
int spi_dwc_ssi_init_interrupt_no_byte_swap (struct spi_dwc_ssi *spi,
	struct spi_dwc_ssi_state *state, struct DWC_ssi_AHB_Slave *regs, struct Ssi_regs *creg,
	uint32_t ssi_clock_hz, uint32_t spi_clock_hz)
{
	int status;

	status = spi_dwc_ssi_init_interrupt (spi, state, regs, creg, ssi_clock_hz, spi_clock_hz);
	if (status != 0) {
		return status;
	}

	spi->no_byte_swap = true;

	return status;
}

/**
 * Initialize only the variable state of the SPI driver instance.  The rest of the driver
 * structure is assumed to have already been initialized.
 *
 * The variable state includes initialization of the hardware.
 *
 * @param spi The SPI driver that contains the state to initialize.
 * @param ssi_clock_hz Frequency of the internal clock for the SPI HW, in Hz.
 * @param spi_clock_hz Desired frequency for the external SPI bus clock, in Hz.  This must be at
 * most half the rate of the internal clock.  The resulting value will be the closest frequency that
 * can be achieved without exceeding the specified SPI clock frequency.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int spi_dwc_ssi_init_state (const struct spi_dwc_ssi *spi, uint32_t ssi_clock_hz,
	uint32_t spi_clock_hz)
{
	int status;

	if ((spi == NULL) || (spi->state == NULL) || (spi->regs == NULL) || (spi->creg == NULL)) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	memset (spi->state, 0, sizeof (struct spi_dwc_ssi_state));

	/* Start the HW in the disabled state.  It will only be enabled when a transaction is requested.
	 * The HW needs to be disabled to write to several control registers. */
	spi->regs->ssic_address_block.SSIENR =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SSIENR_SSIC_EN_SSIC_EN_DISABLE;
	spi->regs->ssic_address_block.IMR = 0;

	/* Set the Tx FIFO level to trigger as soon as the FIFO has free space.  It's critically
	 * important that the Tx FIFO never drain fully, so give FW the most time possible to respond to
	 * interrupts. */
	spi->regs->ssic_address_block.TXFTLR =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_TXFTLR_TFT_SET (SPI_DWC_SSI_TX_FIFO_DEPTH - 1);

	/* Configured as a SPI master.  Setting the baud rate may depend on this. */
	spi->regs->ssic_address_block.CTRLR0 = SPI_DWC_SSI_MASTER;

	/* Configure the SPI bus clock. */
	status = spi_dwc_ssi_calculate_clock_divider (ssi_clock_hz, spi_clock_hz);
	if (status == FLASH_MASTER_FREQ_OUT_OF_RANGE) {
		return status;
	}

	spi->regs->ssic_address_block.BAUDR = status;

	spi->state->ssi_clock = ssi_clock_hz;

	status = platform_semaphore_init (&spi->state->done);
	if (status != 0) {
		return status;
	}

	status = platform_mutex_init (&spi->state->lock);
	if (status != 0) {
		platform_semaphore_free (&spi->state->done);
	}

	return status;
}

/**
 * Release the resources used by a DesignWare SPI driver.
 *
 * @param spi The SPI driver instance to release.
 */
void spi_dwc_ssi_release (const struct spi_dwc_ssi *spi)
{
	if (spi) {
		platform_semaphore_free (&spi->state->done);
		platform_mutex_free (&spi->state->lock);
	}
}

/**
 * Initialize the SPI hardware configuration and other transfer management details.
 *
 * @param xfer The transfer descriptor to execute.
 * @param spi_xfer Internal descriptor for transfer management that should be initialized.
 *
 * @return 0 if the transfer configuration was initialized or an error code.
 */
static int spi_dwc_ssi_init_xfer_config (const struct flash_xfer *xfer,
	struct spi_dwc_ssi_xfer *spi_xfer)
{
	/* Static configuration for all transfers. */
	spi_xfer->ctrlr0 = SPI_DWC_SSI_MASTER;
	spi_xfer->spi_ctrlr0 = SPI_DWC_SSI_8BIT_INSTRUCTION;
#if DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_CLK_STRETCH_EN_WRITE_ACCESS == 1
	spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_ENABLE_CLOCK_STRETCHING;
#endif

	/* Configure the SPI mode for the transfer and any necessary wait cycles.
	 *
	 * Mode bytes are currently handled the same as dummy bytes, which means no specific value is
	 * being output during these clocks other than the constant logic level the HW normally drives.
	 * This is probably fine, since mode bits of 0xAx are used to change from normal operation.  A
	 * constant output, low or high, would not generate this sequence. */
	spi_xfer->is_dual_quad = xfer->flags & (FLASH_FLAG_DUAL_CMD | FLASH_FLAG_DUAL_ADDR |
		FLASH_FLAG_DUAL_DATA | FLASH_FLAG_QUAD_CMD | FLASH_FLAG_QUAD_ADDR | FLASH_FLAG_QUAD_DATA);
	switch (spi_xfer->is_dual_quad) {
		case FLASH_FLAG_DUAL_CMD | FLASH_FLAG_DUAL_ADDR | FLASH_FLAG_DUAL_DATA:
			spi_xfer->ctrlr0 |= SPI_DWC_SSI_DUAL | SPI_DWC_SSI_32BIT_FRAME;
			spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_X_X_X |
				SPI_DWC_SSI_DUAL_WAIT_CYCLES (xfer->mode_bytes + xfer->dummy_bytes);
			break;

		case FLASH_FLAG_DUAL_ADDR | FLASH_FLAG_DUAL_DATA:
			spi_xfer->ctrlr0 |= SPI_DWC_SSI_DUAL | SPI_DWC_SSI_32BIT_FRAME;
			spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_1_X_X |
				SPI_DWC_SSI_DUAL_WAIT_CYCLES (xfer->mode_bytes + xfer->dummy_bytes);
			break;

		case FLASH_FLAG_DUAL_DATA:
			spi_xfer->ctrlr0 |= SPI_DWC_SSI_DUAL | SPI_DWC_SSI_32BIT_FRAME;
			spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_1_1_X |
				SPI_DWC_SSI_SINGLE_WAIT_CYCLES (xfer->mode_bytes + xfer->dummy_bytes);
			break;

		case FLASH_FLAG_QUAD_CMD | FLASH_FLAG_QUAD_ADDR | FLASH_FLAG_QUAD_DATA:
			spi_xfer->ctrlr0 |= SPI_DWC_SSI_QUAD | SPI_DWC_SSI_32BIT_FRAME;
			spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_X_X_X |
				SPI_DWC_SSI_QUAD_WAIT_CYCLES (xfer->mode_bytes + xfer->dummy_bytes);
			break;

		case FLASH_FLAG_QUAD_ADDR | FLASH_FLAG_QUAD_DATA:
			spi_xfer->ctrlr0 |= SPI_DWC_SSI_QUAD | SPI_DWC_SSI_32BIT_FRAME;
			spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_1_X_X |
				SPI_DWC_SSI_QUAD_WAIT_CYCLES (xfer->mode_bytes + xfer->dummy_bytes);
			break;

		case FLASH_FLAG_QUAD_DATA:
			spi_xfer->ctrlr0 |= SPI_DWC_SSI_QUAD | SPI_DWC_SSI_32BIT_FRAME;
			spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_1_1_X |
				SPI_DWC_SSI_SINGLE_WAIT_CYCLES (xfer->mode_bytes + xfer->dummy_bytes);
			break;

		case 0:
			spi_xfer->ctrlr0 |= SPI_DWC_SSI_SINGLE | SPI_DWC_SSI_8BIT_FRAME;
			break;

		default:
			return FLASH_MASTER_UNSUPPORTED_XFER;
	}

#if DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_CLK_STRETCH_EN_WRITE_ACCESS == 0
	if (flash_xfer_is_tx (xfer) && spi_xfer->is_dual_quad) {
		/* Cannot support Dual/Quad Tx transfers without clock stretching available.  There seems to
		 * be a HW limitation causing incorrect behavior when trying to run an enhanced Tx transfer
		 * without any command or address bytes. */
		return FLASH_MASTER_UNSUPPORTED_XFER;
	}
#endif

	/* Configure the address length. */
	if (flash_xfer_uses_4byte_address (xfer)) {
		spi_xfer->addr_bytes = 4;
		spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_4BYTE_ADDRESS;
	}
	else if (flash_xfer_has_no_address (xfer)) {
		spi_xfer->addr_bytes = 0;
		spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_NO_ADDRESS;
	}
	else {
		spi_xfer->addr_bytes = 3;
		spi_xfer->spi_ctrlr0 |= SPI_DWC_SSI_3BYTE_ADDRESS;
	}

	/* Configure the data direction and length. */
	if ((xfer->length == 0) || flash_xfer_is_tx (xfer)) {
		spi_xfer->ctrlr0 |= SPI_DWC_SSI_TX;
	}
	else if (spi_xfer->is_dual_quad == 0) {
		spi_xfer->ctrlr0 |= SPI_DWC_SSI_SINGLE_RX;
	}
	else {
		spi_xfer->ctrlr0 |= SPI_DWC_SSI_MULTI_RX;
	}

	spi_xfer->xfer = xfer;
	spi_xfer->new_xfer = true;

	return 0;
}

/**
 * Determine the length of the next hardware transaction.
 *
 * @param spi The SPI master executing the transfer.
 * @param remaining_bytes The total number of bytes remaining in the overall transfer.
 * @param clock_stretching Flag indicating if clock stretching is supported in the hardware.
 */
static void spi_dwc_ssi_determine_transaction_length (const struct spi_dwc_ssi *spi,
	size_t remaining_bytes, const bool clock_stretching)
{
	struct spi_dwc_ssi_xfer *spi_xfer = spi->state->active;
	const struct flash_xfer *xfer = spi_xfer->xfer;
	uint8_t tx_overhead;
	bool adjust_bytes;

	/* There are several considerations that factor into the decision of transaction length:
	 * 1. For Tx transfers, don't manipulate the length in any way.  Program operations need to all
	 *    happen in one transaction context, so they can't be broken up regardless of length or
	 *    buffer alignment.
	 * 2. Single SPI Rx transfers have no alignment limitations.  Transfer all the bytes in a single
	 *    transaction.
	 * 3. Dual/Quad Rx transfers into a 32-bit aligned buffer don't need any special treatment.
	 *    Transfer all the bytes in a single transaction.
	 * 4. Dual/Quad Rx transfers into an unaligned buffer suffer a performance penalty when reading
	 *    data from the Rx FIFO since it takes multiple instructions to extract the single bytes
	 *    from the 32-bit FIFO entry.  In some cases, such as when traversing the DMB, the
	 *    performance penalty may be so great that the Rx FIFO cannot be drained fast enough.  To
	 *    mitigate this scenario, transfers that are larger than the Rx FIFO capacity will be broken
	 *    into two stages.  First a small transaction utilizing the entire Rx FIFO to achieve 32-bit
	 *    alignment on the output buffer.  Then an aligned transaction for the remaining data.  Each
	 *    transaction with the flash carries command and address overhead, so skipping this aligning
	 *    transaction for small transfers avoids paying that cost unnecessarily.  The Rx FIFO size
	 *    is used as the deciding factor since any transaction that can fit entirely in the FIFO
	 *    will never overflow, regardless of how slowly the FIFO is drained.  Any transaction larger
	 *    than the Rx FIFO carries the possibility of overflow. */
	if ((spi_xfer->is_dual_quad == 0) || flash_xfer_is_tx (xfer) ||
		SPI_DWC_SSI_IS_UINT32_ALIGNED (spi_xfer->data) ||
		(remaining_bytes <= (SPI_DWC_SSI_RX_FIFO_DEPTH * 4))) {
		/* The transfer falls into one of the cases that don't require length adjustments. */
		spi_xfer->rx_alignment = 0;
		spi_xfer->xfer_bytes = remaining_bytes;
	}
	else {
		/* Need to execute an abbreviated transaction first to achieve buffer alignment. */
		spi_xfer->rx_alignment = ((uintptr_t) spi_xfer->data) & 0x03;
		spi_xfer->xfer_bytes = (SPI_DWC_SSI_RX_FIFO_DEPTH * 4) - spi_xfer->rx_alignment;
	}

	tx_overhead = 0;
	spi_xfer->xfer_frames = 1;
	spi_xfer->xfer_start = 1;
	adjust_bytes = false;

	/* Once the desired transaction length has been decided, it further needs to be checked for
	 * compatibility with the HW.  The final transaction length will be adjusted as needed.
	 *
	 * No additional checks are necessary for commands that don't send or receive any data bytes. */
	if (spi_xfer->xfer_bytes != 0) {
		/* Determine how many Tx FIFO entries are needed for command overhead. */
		if (spi_xfer->new_xfer) {
			/* The command byte uses one FIFO entry. */
			tx_overhead++;

			if (spi_xfer->is_dual_quad) {
				/* Dual/Quad mode needs a single FIFO entry for the address. */
				if (spi_xfer->addr_bytes) {
					tx_overhead++;
				}

				/* Hardware handles dummy bytes for Dual/Quad transactions without additional FIFO
				 * entries. */
			}
			else {
				/* Single SPI needs one FIFO entry for each address and dummy byte. */
				tx_overhead += spi_xfer->addr_bytes + xfer->mode_bytes + xfer->dummy_bytes;
			}
		}

		/* Determine how many data frames will be transferred. */
		if (spi_xfer->is_dual_quad) {
			/* Dual/Quad transfers must be multiples of 4 bytes. */
			if (flash_xfer_is_tx (xfer) && (spi_xfer->xfer_bytes & 0x3)) {
				/* Unaligned transfers that send data to flash cannot be supported. */
				spi_xfer->xfer_status = FLASH_MASTER_UNSUPPORTED_XFER;

				return;
			}

			/* For receive transfers, just read an extra word to achieve length alignment. */
			spi_xfer->xfer_frames = (spi_xfer->xfer_bytes + 3) / 4;
		}
		else {
			/* Single SPI transfers can transfer any number of bytes. */
			spi_xfer->xfer_frames = spi_xfer->xfer_bytes;
		}

		/* The HW only supports a max of 64kB frames per transfer.  This is only really needed when
		 * clock stretching is available, since FIFO sizes will otherwise be the limiting factor. */
#if DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_CLK_STRETCH_EN_WRITE_ACCESS == 1
		if (spi_xfer->xfer_frames > SPI_DWC_SSI_MAX_FRAMES) {
			spi_xfer->xfer_frames = SPI_DWC_SSI_MAX_FRAMES;
			adjust_bytes = true;
		}
#endif

		/* For single SPI and HW that doesn't support clock stretching, limit transfers to the size
		 * of the Rx/Tx FIFOs to avoid the possibility of over/underflow. */
		if (flash_xfer_is_tx (xfer)) {
			if (!clock_stretching || !spi_xfer->is_dual_quad) {
				if ((tx_overhead + spi_xfer->xfer_frames) > SPI_DWC_SSI_TX_FIFO_DEPTH) {
					spi_xfer->xfer_frames = SPI_DWC_SSI_TX_FIFO_DEPTH - tx_overhead;
					adjust_bytes = true;
				}
			}

			/* For Tx transfers, specify the Tx FIFO level required before the HW should start
			 * executing the transaction.  This will prevent FIFO underflow by ensuring all data is
			 * present in the FIFO before any data starts to be sent. */
			spi_xfer->xfer_start = spi_xfer->xfer_frames + tx_overhead;
			if (spi_xfer->xfer_start > SPI_DWC_SSI_TX_FIFO_DEPTH) {
				spi_xfer->xfer_start = SPI_DWC_SSI_TX_FIFO_DEPTH;
			}
		}
		else if (!clock_stretching || !spi_xfer->is_dual_quad) {
			/* Rx transfers don't need to account for the command overhead since data will be
			 * received in a different FIFO. */
			if (spi_xfer->xfer_frames > SPI_DWC_SSI_RX_FIFO_DEPTH) {
				spi_xfer->xfer_frames = SPI_DWC_SSI_RX_FIFO_DEPTH;
				adjust_bytes = true;
			}

			/* Rx transfers don't need to be concerned with Tx FIFO underflow.  All necessary data
			 * will be in the FIFO before the slave select is set to start the transaction. */
		}

		/* Update the number of data bytes for this transfer if a length modification was
		 * necessary. */
		if (adjust_bytes) {
			spi_xfer->xfer_bytes = spi_xfer->xfer_frames;

			if (spi_xfer->is_dual_quad) {
				spi_xfer->xfer_bytes *= 4;
			}
		}
	}
}

/**
 * Start a hardware transaction in the overall transfer to flash.
 *
 * @param spi The SPI master executing the transfer.
 * @param slave_select Index for the slave select signal being used for the transfer.
 * @param clock_stretching Flag indicating if clock stretching is supported in the hardware.
 */
static void spi_dwc_ssi_start_transaction (const struct spi_dwc_ssi *spi, uint8_t slave_select,
	const bool clock_stretching)
{
	struct spi_dwc_ssi_xfer *spi_xfer = spi->state->active;
	const struct flash_xfer *xfer = spi_xfer->xfer;
	uint8_t addr[4] = {0};
	int i;

	/* Apply the configuration and enable SPI HW so the FIFOs can be accessed. */
	spi->regs->ssic_address_block.BAUDR = spi_xfer->clk_divider;
	spi->regs->ssic_address_block.CTRLR0 = spi_xfer->ctrlr0;
	spi->regs->ssic_address_block.CTRLR1 = spi_xfer->xfer_frames - 1;
	spi->regs->ssic_address_block.SPI_CTRLR0 = spi_xfer->spi_ctrlr0;
	spi->regs->ssic_address_block.SER = 0;
	spi->regs->ssic_address_block.TXFTLR =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_TXFTLR_TXFTHR_MODIFY (spi_xfer->txftlr,
		spi_xfer->xfer_start - 1);
	spi->regs->ssic_address_block.SSIENR =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SSIENR_SSIC_EN_SSIC_EN_ENABLED;

	/* Clear any transfer error status. */
	spi_dwc_ssi_clear_interrupt (&spi->regs->ssic_address_block.TXEICR);
	spi_dwc_ssi_clear_interrupt (&spi->regs->ssic_address_block.RXUICR);

	/* Load the Tx FIFO with command, address, and dummy data. */
	if (spi_xfer->new_xfer) {
		spi->regs->ssic_address_block.DR0 = xfer->cmd;

		if (spi_xfer->addr_bytes) {
			if (spi_xfer->is_dual_quad) {
				/* For Dual/Quad transfers, the entire address goes in a single FIFO entry.  Proper
				 * endianness is handled by the HW. */
				spi->regs->ssic_address_block.DR0 = xfer->address + spi_xfer->sent_bytes;
			}
			else {
				/* For single SPI transfers, each address byte gets queued separately. */
				flash_int_to_address (xfer->address + spi_xfer->sent_bytes, spi_xfer->addr_bytes,
					addr);

				spi->regs->ssic_address_block.DR0 = addr[0];
				spi->regs->ssic_address_block.DR0 = addr[1];
				spi->regs->ssic_address_block.DR0 = addr[2];
				if (spi_xfer->addr_bytes == 4) {
					spi->regs->ssic_address_block.DR0 = addr[3];
				}
			}
		}

		if (!spi_xfer->is_dual_quad) {
			/* Single SPI doesn't use the WAIT_CYCLES register setting, so just fill the Tx FIFO
			 * with extra bytes.  There is no checking for available space on the Tx FIFO here
			 * because this is really only ever going to be 0 or 1 bytes of data.  The minimum FIFO
			 * depth is 8, which is enough to hold one command byte, 4 address bytes, and a dummy
			 * byte. */
			for (i = 0; i < (xfer->mode_bytes + xfer->dummy_bytes); i++) {
				spi->regs->ssic_address_block.DR0 = 0xff;
			}
		}
	}
	else if (!flash_xfer_is_tx (xfer)) {
		/* Even when no data needs to be sent, there needs to be data in the FIFO to start the
		 * transaction. */
		spi->regs->ssic_address_block.DR0 = 0xff;
	}

	/* Use the CREG override for slave select output to avoid FIFO over/underflow issues.  This will
	 * keep the flash slave select asserted through the entire transaction processing by decoupling
	 * slave select assertion from the individual transactions executed by hardware.
	 *
	 * Without clock stretching support, all SPI transactions need to control the SPI slave select
	 * in this manner.  With clock stretching, Dual/Quad SPI transactions can use the slave select
	 * directly from the hardware block, since the FIFOs will not be allowed to over/underflow. */
	if (!clock_stretching || !spi_xfer->is_dual_quad) {
		if (spi_xfer->new_xfer) {
			spi->creg->SPI_SLAVE_SELECT = SPI_DWC_SSI_CREG_CS_ASSERT (slave_select);
		}
	}
	else {
		spi->creg->SPI_SLAVE_SELECT = 0;
	}

	/* Set the chip select to start the transfer. */
	spi->regs->ssic_address_block.SER = SPI_DWC_SSI_CS (slave_select);
}

/**
 * Determine how to configure the next hardware transaction.
 *
 * @param spi The SPI master executing the transfer.
 * @param slave_select Index for the slave select signal being used for the transfer.
 * @param clock_stretching Flag indicating if clock stretching is supported in the hardware.
 */
static void spi_dwc_ssi_next_transaction_config (const struct spi_dwc_ssi *spi,
	uint8_t slave_select, const bool clock_stretching)
{
	struct spi_dwc_ssi_xfer *spi_xfer = spi->state->active;
	const struct flash_xfer *xfer = spi_xfer->xfer;

	/* Disable the HW to be ready for the next transfer. */
	spi->regs->ssic_address_block.SSIENR =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SSIENR_SSIC_EN_SSIC_EN_DISABLE;

	spi_xfer->sent_bytes += spi_xfer->xfer_bytes;

	if (!clock_stretching || !spi_xfer->is_dual_quad) {
		/* If this was a transfer to achieve word alignment, start a new one on the next pass. */
		spi_xfer->new_xfer = (spi_xfer->rx_alignment != 0);

		if (!spi_xfer->new_xfer) {
			if (!flash_xfer_is_tx (xfer) && !spi_xfer->is_dual_quad) {
				/* For subsequent single SPI Rx transfers, the transfer mode needs to be switched to
				 * Rx-only mode to skip sending command and address data. */
				spi_xfer->ctrlr0 =
					DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_TMOD_MODIFY (spi_xfer->ctrlr0,
					DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_CTRLR0_TMOD_TMOD_RX_ONLY);
			}

			/* Clear the bits indicating that command, address, and dummy bytes should be sent as
			 * part of the transfer.  This is only really applicable to Dual/Quad transfers, but is
			 * harmless to do in all scenarios. */
			spi_xfer->spi_ctrlr0 = DWC_SPI_SSI_XFER_DATA_ONLY (spi_xfer->spi_ctrlr0);
		}
		else {
			/* This is the end of the current transfer, so deassert the slave select. */
			spi->creg->SPI_SLAVE_SELECT = SPI_DWC_SSI_CREG_CS_DEASSERT (slave_select);
		}
	}
	else {
		/* When not using the CREG slave select, always start a new transfer. */
		spi_xfer->new_xfer = true;
	}
}

/**
 * Execute a SPI flash transfer.
 *
 * @param spi The SPI master instance to use for the transfer.
 * @param xfer The transfer to execute.
 * @param slave_select Index for the slave select signal to use.
 * @param spi_clock_hz Desired frequency for the external SPI bus clock, in Hz.  This must be at
 * most half the rate of the internal clock.  The resulting value will be the closest frequency that
 * can be achieved without exceeding the specified SPI clock frequency.
 *
 * @return 0 if the transfer was successfully executed or an error code.
 */
int spi_dwc_ssi_xfer (const struct spi_dwc_ssi *spi, const struct flash_xfer *xfer,
	uint8_t slave_select, uint32_t spi_clock_hz)
{
	struct spi_dwc_ssi_xfer spi_xfer;
	const bool clock_stretching =
		(DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_CLK_STRETCH_EN_WRITE_ACCESS == 1);

	if ((spi == NULL) || (xfer == NULL)) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	memset (&spi_xfer, 0, sizeof (spi_xfer));

	spi_xfer.clk_divider = spi_dwc_ssi_calculate_clock_divider (spi->state->ssi_clock,
		spi_clock_hz);
	if (spi_xfer.clk_divider == FLASH_MASTER_FREQ_OUT_OF_RANGE) {
		return spi_xfer.clk_divider;
	}

	spi_xfer.xfer_status = spi_dwc_ssi_init_xfer_config (xfer, &spi_xfer);
	if (spi_xfer.xfer_status != 0) {
		return spi_xfer.xfer_status;
	}

	platform_mutex_lock (&spi->state->lock);
	spi->state->active = &spi_xfer;
	spi_xfer.txftlr = spi->regs->ssic_address_block.TXFTLR;

	do {
		spi_xfer.xfer_status = spi->prepare_transaction (spi);
		if (spi_xfer.xfer_status != 0) {
			goto exit;
		}

		spi_xfer.data = &xfer->data[spi_xfer.sent_bytes];
		spi_dwc_ssi_determine_transaction_length (spi, xfer->length - spi_xfer.sent_bytes,
			clock_stretching);
		if (spi_xfer.xfer_status != 0) {
			goto exit;
		}

		spi_dwc_ssi_start_transaction (spi, slave_select, clock_stretching);

		/* Read/Write the data portion of the command. */
		if (spi_xfer.xfer_bytes) {
			spi->handle_transaction_data (spi);
			if (spi_xfer.xfer_status != 0) {
				goto exit;
			}
		}

		/* If this is a Tx transfer, wait until all the data has been sent.  If this is an Rx
		 * transfer, all the requested data was received before reaching this point, so there is no
		 * need for additional waiting. */
		if ((spi_xfer.xfer_bytes == 0) || flash_xfer_is_tx (xfer)) {
			uint32_t sr;

			do {
				sr = spi->regs->ssic_address_block.SR;
			} while ((sr & SPI_DWC_SSI_REG_BIT_MASK (SR, BUSY)) ||
				!(sr & SPI_DWC_SSI_REG_BIT_MASK (SR, TFE)));
		}

		spi_dwc_ssi_next_transaction_config (spi, slave_select, clock_stretching);
	} while (spi_xfer.sent_bytes != xfer->length);

exit:
	/* Also disable the HW here to account for error cases that escape the loop early. */
	spi->regs->ssic_address_block.SSIENR =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SSIENR_SSIC_EN_SSIC_EN_DISABLE;

	/* Terminate the the transaction.  This is only necessary if the CREG slave select override was
	 * used for the transaction.*/
	if (spi->creg->SPI_SLAVE_SELECT != 0) {
		spi->creg->SPI_SLAVE_SELECT = SPI_DWC_SSI_CREG_CS_DEASSERT (slave_select);
	}

	spi->state->active = NULL;

	platform_mutex_unlock (&spi->state->lock);

	return spi_xfer.xfer_status;
}

/**
 * Get the currently configured frequency being used for SPI transactions.
 *
 * @param spi The SPI master instance to query.
 *
 * @return The current SPI clock frequency, in Hz, or an error code if the instance is not valid.
 */
int spi_dwc_ssi_get_spi_frequency (const struct spi_dwc_ssi *spi)
{
	if (spi == NULL) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	return spi->state->ssi_clock / spi->regs->ssic_address_block.BAUDR;
}

/**
 * Set the frequency to be used for SPI transactions.  This will only possible when the SPI master
 * is not actively transmitting.
 *
 * @param spi The SPI master instance to configure.
 * @param spi_clock_hz Desired frequency for the external SPI bus clock, in Hz.  This must be at
 * most half the rate of the internal clock.  The resulting value will be the closest frequency that
 * can be achieved without exceeding the specified SPI clock frequency.
 *
 * @return 0 if the frequency was configured successfully or an error code.
 */
int spi_dwc_ssi_set_spi_frequency (const struct spi_dwc_ssi *spi, uint32_t spi_clock_hz)
{
	int status;

	if (spi == NULL) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	if (spi->regs->ssic_address_block.SSIENR) {
		return FLASH_MASTER_XFER_IN_PROGRESS;
	}

	platform_mutex_lock (&spi->state->lock);

	status = spi_dwc_ssi_calculate_clock_divider (spi->state->ssi_clock, spi_clock_hz);
	if (status != FLASH_MASTER_FREQ_OUT_OF_RANGE) {
		spi->regs->ssic_address_block.BAUDR = status;
	}

	platform_mutex_unlock (&spi->state->lock);

	return (status == FLASH_MASTER_FREQ_OUT_OF_RANGE) ? status :
			   (int) (spi->state->ssi_clock / spi->regs->ssic_address_block.BAUDR);
}

/**
 * Determine the closest supported frequency to the desired target.  This would be the frequency
 * that would be used by the SPI hardware if it was configured for the target frequency, but no
 * hardware settings will be impacted.
 *
 * @param spi The SPI master instance to query.
 * @param spi_clock_hz Desired frequency for the external SPI bus clock, in Hz.  This must be at
 * most half the rate of the internal clock.
 *
 * @return The nearest frequency achievable fo the SPI hardware, in Hz, or an error code.
 */
int spi_dwc_ssi_get_supported_frequency (const struct spi_dwc_ssi *spi, uint32_t spi_clock_hz)
{
	int clk_divider;

	if (spi == NULL) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	clk_divider = spi_dwc_ssi_calculate_clock_divider (spi->state->ssi_clock, spi_clock_hz);

	return (clk_divider == FLASH_MASTER_FREQ_OUT_OF_RANGE) ?
			   clk_divider : (int) (spi->state->ssi_clock / clk_divider);
}

/**
 * Get the currently configured rx_sample_delay being used for SPI transactions.
 *
 * @param spi The SPI master instance to query.
 *
 * @return The current rx_sample_delay, or an error code if the instance is not valid.
 */
int spi_dwc_ssi_get_rx_sample_delay (const struct spi_dwc_ssi *spi)
{
	if (spi == NULL) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	return DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_RX_SAMPLE_DELAY_RSD_GET (
		spi->regs->ssic_address_block.RX_SAMPLE_DELAY);
}

/**
 * Set the rx_sample_delay of the SPI driver instance. This is only possible when
 * DWC_SSI is not enabled / not actively transmitting.
 *
 * @param spi The SPI driver that contains the state to initialize.
 * @param rx_sample_delay The value used to program the rx sample delay for the SPI driver
 *
 * @return 0 if the rx_sample_delay was successfully initialized or an error code.
 */
int spi_dwc_ssi_set_rx_sample_delay (const struct spi_dwc_ssi *spi, uint32_t rx_sample_delay)
{
	if (spi == NULL) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	if (spi->regs->ssic_address_block.SSIENR) {
		return FLASH_MASTER_XFER_IN_PROGRESS;
	}

	platform_mutex_lock (&spi->state->lock);

	spi->regs->ssic_address_block.RX_SAMPLE_DELAY =
		DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_RX_SAMPLE_DELAY_RSD_SET (rx_sample_delay);

	platform_mutex_unlock (&spi->state->lock);

	return 0;
}
