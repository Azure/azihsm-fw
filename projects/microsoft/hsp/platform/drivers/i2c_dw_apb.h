// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_H_
#define I2C_DW_APB_H_

#include <stdint.h>
#include "platform_api.h"
#include "drivers/i2c_dw_apb_handler.h"
#include "trap/hsp_interrupt_handler.h"


/**
 * Friendly accessor for Tx Abort Source bits.  This can be used to check bits for the abort source
 * reported by i2c_dw_apb_get_last_tx_abort_source().
 */
#define I2C_DW_APB_TX_ABRT_SOURCE(field) \
	CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_TX_ABRT_SOURCE_ ## field ## _FIELD_MASK


/**
 * The different speed modes the device can be configured to support.
 */
enum i2c_dw_apb_speed {
	I2C_DW_APB_SPEED_STANDARD = 1,	/**< Standard I2C speed up to 100kHz. */
	I2C_DW_APB_SPEED_FAST = 2,		/**< Fast (400kHz) and Fast Plus (1MHz) I2C speed. */
	I2C_DW_APB_SPEED_HIGH = 3,		/**< High I2C speed up to 3.4MHz. */
};

/**
 * The capabilities the driver implementation can support.
 */
enum i2c_dw_apb_capabilities {
	I2C_DW_APB_CAPABILITIES_SLAVE_MODE = 1 << 0,	/**< Driver supports SLAVE mode operations. */
	I2C_DW_APB_CAPABILITIES_MASTER_MODE = 1 << 1,	/**< Driver supports MASTER mode operations. */

	I2C_DW_APB_CAPABILITIES_MULTIMASTER_MODE =		/**< Driver supports MULTIMASTER mode operations. */
		I2C_DW_APB_CAPABILITIES_SLAVE_MODE | I2C_DW_APB_CAPABILITIES_MASTER_MODE,
	I2C_DW_APB_CAPABILITIES_MODE_MASK =				/**< Capabilities mask to determine the driver type. */
		I2C_DW_APB_CAPABILITIES_MULTIMASTER_MODE,
};

/**
 * The current I2C operation modes the HW can be configured for.
 */
enum i2c_dw_apb_mode {
	I2C_DW_APB_MODE_SLAVE,	/**< Hardware is configured for SLAVE mode. */
	I2C_DW_APB_MODE_MASTER,	/**< Hardware is configured for MASTER mode. */
};

/* Defined in HSP register definition.
 *
 * While this register set has the _Slave postfix to it, the hardware is not just a slave device
 * and is named "DW_apb_i2c in it's documentation.  It has the capability to operate in MASTER
 * mode. */
struct Creg_regs_DW_apb_i2c_APB_Slave;

/**
 * Variable context for the I2C driver.
 */
struct i2c_dw_apb_state {
	platform_mutex lock;	/**< I2C driver lock. */
	size_t tx_len;			/**< The length of data left for transmission. */
	const uint8_t *tx_data;	/**< A pointer to data for transmission. */
	size_t rx_pos;			/**< The current position within the Rx buffer. */
	size_t rx_len;			/**< The length of the Rx buffer. */
	uint8_t *rx_buffer;		/**< A pointer to the Rx buffer. */
	uint32_t tx_abort_src;	/**< Status indicating the source of the last Tx Abort interrupt. */
};

/**
 * Base driver instance for supporting I2C operations using the DesignWare APB HW block from
 * Synopsis.
 */
struct i2c_dw_apb {
	struct hsp_interrupt_handler isr_handler;	/**< Interrupt handler implementation. */

	/**
	 * Callback when the driver has successfully shutdown.
	 *
	 * This may be called in a USER context and must be handled.
	 *
	 * @param handler The calling I2C driver instance.
	 */
	void (*on_shutdown) (const struct i2c_dw_apb *i2c);

	/**
	 * A START condition was detected on the bus.  This is before any address has been sent, so
	 * it is not possible to know if this START condition is directed at the local device.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param i2c The calling I2C driver instance.
	 *
	 * @return An interrupt mask to detect initial transaction events.
	 */
	uint32_t (*on_start) (const struct i2c_dw_apb *i2c);

	/**
	 * Callback when there is data in the Rx FIFO and there is no buffer available to write
	 * into.
	 *
	 * If a valid buffer isn't set after this callback, the driver will discard the transaction.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param i2c The calling I2C driver instance.
	 */
	void (*on_rx_full) (const struct i2c_dw_apb *i2c);

	/**
	 * Callback when the Rx FIFO has been overflown or no buffer available to read into.  The
	 * following Rx data will be discarded until a STOP or RESTART condition occurs.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param i2c The calling I2C driver instance.
	 */
	void (*on_rx_discard) (const struct i2c_dw_apb *i2c);

	/**
	 * Callback when a MASTER is requesting data from this I2C instance.
	 *
	 * If a valid buffer is not queued after this callback, the driver will automatically stall the
	 * HW until TX ABORT, STOP, or RESTART interrupt happens.  It is up to the upper implementation
	 * layer to queue a buffer and enable the RD_REQ interrupt via a driver mode API call that
	 * implements driver_try_read_request.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed,
	 * as this only makes sense in a SLAVE mode operation.
	 *
	 * @param i2c The calling I2C driver instance.
	 */
	void (*on_read_request) (const struct i2c_dw_apb *i2c);

	/**
	 * Callback when the current transmission is being aborted.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed.
	 *
	 * @param i2c The calling I2C driver instance.
	 */
	void (*on_tx_abort) (const struct i2c_dw_apb *i2c);

	/**
	 * Callback when SCL has been held low for the configured amount of time.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed.
	 *
	 * @param i2c The calling I2C driver instance.
	 */
	void (*on_tx_timeout) (const struct i2c_dw_apb *i2c);

	/**
	 * Callback when all the data in the current Tx buffer has been pushed to the FIFO.  This does
	 * not indicate that all the data in the FIFO has been received by the other endpoint.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed.
	 *
	 * @param i2c The calling I2C driver instance.
	 */
	void (*on_tx_depleted) (const struct i2c_dw_apb *i2c);

	/**
	 * A RESTART condition was detected on the bus while the local device is being addressed.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param i2c The calling driver instance.
	 *
	 * @return An interrupt mask to detect the next event.
	 */
	uint32_t (*on_restart) (const struct i2c_dw_apb *i2c);

	/**
	 * A STOP condition was detected on the bus while the local device is being addressed.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param i2c The calling driver instance.
	 */
	void (*on_stop) (const struct i2c_dw_apb *i2c);

	struct i2c_dw_apb_state *state;					/**< Variable context for the driver. */
	struct Creg_regs_DW_apb_i2c_APB_Slave *regs;	/**< Register interface for the I2C HW. */
	const struct i2c_dw_apb_handler *i2c_handler;	/**< Event handler for the I2C device. */
	int capabilities;								/**< Capability mask that the driver implementation supports. */
};


/* Public API */

size_t i2c_dw_apb_rx_discard_buffer (uint8_t **data);
size_t i2c_dw_apb_tx_pad_buffer (const uint8_t **data);

int i2c_dw_apb_get_tx_fifo_depth (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_get_mode (const struct i2c_dw_apb *i2c);
bool i2c_dw_apb_is_hw_enabled (const struct i2c_dw_apb *i2c);
bool i2c_dw_apb_is_active (const struct i2c_dw_apb *i2c);
bool i2c_dw_apb_tx_fifo_not_empty (const struct i2c_dw_apb *i2c);
bool i2c_dw_apb_is_tx_data_pending (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_get_slave_address (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_get_target_address (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_get_address_for_mode (const struct i2c_dw_apb *i2c);

int i2c_dw_apb_lock_driver (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_unlock_driver (const struct i2c_dw_apb *i2c);

int i2c_dw_apb_abort_tx (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_abort_tx_and_wait (const struct i2c_dw_apb *i2c);
uint32_t i2c_dw_apb_get_last_tx_abort_source (const struct i2c_dw_apb *i2c);

int i2c_dw_apb_shutdown_hw (const struct i2c_dw_apb *i2c);

int i2c_dw_apb_try_handle_transaction (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_handle_transaction (const struct i2c_dw_apb *i2c);

int i2c_dw_apb_enable_slave_mode (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_enable_slave_mode_isr (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_begin_master_transmit (const struct i2c_dw_apb *i2c, uint8_t target_address,
	const uint8_t *data, size_t len);

/* Internal functions for use by derived types. */

uint32_t i2c_dw_apb_slave_start_intr_mask ();
uint32_t i2c_dw_apb_tx_intr_mask ();

int i2c_dw_apb_init (struct i2c_dw_apb *i2c, struct i2c_dw_apb_state *state,
	struct Creg_regs_DW_apb_i2c_APB_Slave *regs, const struct i2c_dw_apb_handler *handler,
	int capabilities);
int i2c_dw_apb_init_state (const struct i2c_dw_apb *i2c);
void i2c_dw_apb_release (const struct i2c_dw_apb *i2c);

int i2c_dw_apb_init_hw (const struct i2c_dw_apb *i2c, enum i2c_dw_apb_speed mode, uint32_t ic_clk);
int i2c_dw_apb_configure_slave_mode (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_set_slave_address (const struct i2c_dw_apb *i2c, uint8_t address);
int i2c_dw_apb_set_scl_timeout (const struct i2c_dw_apb *i2c, uint32_t ic_clk, unsigned timeout_ms);

int i2c_dw_apb_try_read_request (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_set_tx_data (const struct i2c_dw_apb *i2c, const uint8_t *data, size_t len);
int i2c_dw_apb_transmit_default_padding_byte (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_set_rx_buffer (const struct i2c_dw_apb *i2c, uint8_t *buffer, size_t len);
int i2c_dw_apb_discard_rx (const struct i2c_dw_apb *i2c);

void i2c_dw_apb_call_on_shutdown (const struct i2c_dw_apb *i2c);
void i2c_dw_apb_call_rx_discard (const struct i2c_dw_apb *i2c);
void i2c_dw_apb_call_rx_complete (const struct i2c_dw_apb *i2c);
void i2c_dw_apb_try_call_tx_abort (const struct i2c_dw_apb *i2c);
void i2c_dw_apb_try_call_tx_complete (const struct i2c_dw_apb *i2c);
int i2c_dw_apb_slave_rx_call_rx_pending (const struct i2c_dw_apb *i2c);
void i2c_dw_apb_slave_rx_call_rx_data (const struct i2c_dw_apb *i2c);


#endif	/* I2C_DW_APB_H_ */
