// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/unused.h"
#include "drivers/i2c_dw_apb_error.h"
#include "recovery/i2c_dw_apb_slave_observer_ocp_recovery.h"


/**
 * State machine for OCP recovery transactions.
 */
enum i2c_dw_apb_slave_observer_txn_state {
	I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_IDLE,	/**< No transaction is active. */
	I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_RX,	/**< Currently receiving data. */
	I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_TX,	/**< Transmitting data. */
	I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_PAD,	/**< Servicing read requests with pad bytes. */
};


/**
 * Called when the end of a transaction has occurred and resets the state.
 *
 * @param recovery The OCP recovery instance.
 */
static void i2c_dw_apb_slave_observer_ocp_recovery_stop (
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery)
{
	ocp_recovery_smbus_stop (recovery->smbus);
	recovery->state->txn_state = I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_IDLE;
}

void i2c_dw_apb_slave_observer_ocp_recovery_on_shutdown (
	const struct i2c_dw_apb_slave_observer_handler *handler)
{
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery =
		(const struct i2c_dw_apb_slave_observer_ocp_recovery*) handler;

	recovery->state->txn_state = I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_IDLE;
}

size_t i2c_dw_apb_slave_observer_ocp_recovery_on_rx_pending (
	const struct i2c_dw_apb_slave_observer_handler *handler, uint16_t address, uint8_t **data)
{
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery =
		(const struct i2c_dw_apb_slave_observer_ocp_recovery*) handler;

	/* The DesignWare I2C hardware has no notification when a device is specifically addressed.
	 * The first byte that is written indicates the start of a new transaction for this device. */
	if (recovery->state->txn_state == I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_IDLE) {
		ocp_recovery_smbus_start (recovery->smbus, (uint8_t) address);
		recovery->state->txn_state = I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_RX;
	}

	recovery->state->rx_data = 0xFF;
	*data = &recovery->state->rx_data;

	return 1;
}

void i2c_dw_apb_slave_observer_ocp_recovery_on_rx_data (
	const struct i2c_dw_apb_slave_observer_handler *handler, size_t count)
{
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery =
		(const struct i2c_dw_apb_slave_observer_ocp_recovery*) handler;

	if (count == 0) {
		return;
	}

	/* The DesignWare I2C hardware doesn't provide a clean way to generate a NACK when receiving
	 * data as a slave device.  Since we can't force the NACK, we don't care about the return value
	 * of this call. */
	ocp_recovery_smbus_receive_byte (recovery->smbus, recovery->state->rx_data);
}

void i2c_dw_apb_slave_observer_ocp_recovery_on_rx_discard (
	const struct i2c_dw_apb_slave_observer_handler *handler)
{
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery =
		(const struct i2c_dw_apb_slave_observer_ocp_recovery*) handler;

	i2c_dw_apb_slave_observer_ocp_recovery_stop (recovery);
}

void i2c_dw_apb_slave_observer_ocp_recovery_on_rx_complete (
	const struct i2c_dw_apb_slave_observer_handler *handler)
{
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery =
		(const struct i2c_dw_apb_slave_observer_ocp_recovery*) handler;

	i2c_dw_apb_slave_observer_ocp_recovery_stop (recovery);
}

size_t i2c_dw_apb_slave_observer_ocp_recovery_on_read_request (
	const struct i2c_dw_apb_slave_observer_handler *handler, uint16_t address, const uint8_t **data)
{
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery =
		(const struct i2c_dw_apb_slave_observer_ocp_recovery*) handler;
	const union ocp_recovery_smbus_cmd_buffer *tx = NULL;

	if (recovery->state->txn_state == I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_PAD) {
		return 0;
	}

	recovery->state->txn_state = I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_TX;

	/* The SMBus handler will generate a single response message per I2C transaction, which will
	 * be fully provided to the I2C driver.  If the driver comes back for more data because the
	 * remote master continues to request data, a padding byte will be sent until the master
	 * stops requesting data. */
	ocp_recovery_smbus_transmit_bytes (recovery->smbus, (uint8_t) address, &tx);

	*data = tx->bytes;

	return tx->block_cmd.byte_count + 2;
}

void i2c_dw_apb_slave_observer_ocp_recovery_on_tx_depleted (
	const struct i2c_dw_apb_slave_observer_handler *handler)
{
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery =
		(const struct i2c_dw_apb_slave_observer_ocp_recovery*) handler;

	recovery->state->txn_state = I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_TXN_STATE_PAD;
}

void i2c_dw_apb_slave_observer_ocp_recovery_on_tx_complete (
	const struct i2c_dw_apb_slave_observer_handler *handler)
{
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery =
		(const struct i2c_dw_apb_slave_observer_ocp_recovery*) handler;

	i2c_dw_apb_slave_observer_ocp_recovery_stop (recovery);
}

/**
 * Initialize an OCP recovery handler for I2C slave events.
 *
 * @param recovery The I2C handler to initialize.
 * @param state Variable context for the I2C handler.  This must not already be initialized.
 * @param smbus SMBus handler for OCP recovery commands.
 *
 * @return 0 if the I2C handler was successfully initialized or an error code.
 */
int i2c_dw_apb_slave_observer_ocp_recovery_init (
	struct i2c_dw_apb_slave_observer_ocp_recovery *recovery,
	struct i2c_dw_apb_slave_observer_ocp_recovery_state *state,
	const struct ocp_recovery_smbus *smbus)
{
	if ((recovery == NULL) || (state == NULL) || (smbus == NULL)) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	memset (recovery, 0, sizeof (struct i2c_dw_apb_slave_observer_ocp_recovery));

	recovery->i2c_handler.on_shutdown = i2c_dw_apb_slave_observer_ocp_recovery_on_shutdown;
	recovery->i2c_handler.on_rx_pending = i2c_dw_apb_slave_observer_ocp_recovery_on_rx_pending;
	recovery->i2c_handler.on_rx_discard = i2c_dw_apb_slave_observer_ocp_recovery_on_rx_discard;
	recovery->i2c_handler.on_rx_data = i2c_dw_apb_slave_observer_ocp_recovery_on_rx_data;
	recovery->i2c_handler.on_rx_complete = i2c_dw_apb_slave_observer_ocp_recovery_on_rx_complete;
	recovery->i2c_handler.on_read_request = i2c_dw_apb_slave_observer_ocp_recovery_on_read_request;
	recovery->i2c_handler.on_tx_depleted = i2c_dw_apb_slave_observer_ocp_recovery_on_tx_depleted;
	recovery->i2c_handler.on_tx_complete = i2c_dw_apb_slave_observer_ocp_recovery_on_tx_complete;

	recovery->state = state;
	recovery->smbus = smbus;

	return i2c_dw_apb_slave_observer_ocp_recovery_init_state (recovery);
}

/**
 * Initialize only the variable state of an OCP recovery I2C slave handler.  The rest of the I2C
 * handler structure is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized handler instance.
 *
 * @param recovery The I2C handler that contains the state to initialize.
 *
 * @return 0 if the handler state was successfully initialized or an error code.
 */
int i2c_dw_apb_slave_observer_ocp_recovery_init_state (
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery)
{
	if ((recovery == NULL) || (recovery->state == NULL)) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	memset (recovery->state, 0, sizeof (struct i2c_dw_apb_slave_observer_ocp_recovery_state));

	return 0;
}

/**
 * Release the resources used by an OCP recovery I2C slave handler.
 *
 * @param recovery The I2C handler to release.
 */
void i2c_dw_apb_slave_observer_ocp_recovery_release (
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery)
{
	UNUSED (recovery);
}
