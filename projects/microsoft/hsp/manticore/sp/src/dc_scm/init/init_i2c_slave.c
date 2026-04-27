// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdint.h>
#include "hsp_top.h"
#include "init/init_cmd.h"
#include "init/init_i2c_slave.h"
#include "trap/hsp_interrupt.h"

/**
 * Forward declaration for system_i2c.
 */
const struct i2c_dw_apb_slave i2c_hw;

/**
 * Variable context for the I2C command channel.
 */
struct cmd_channel_i2c_dw_apb_slave_state system_i2c_context;

/**
 * The I2C interface to the BMC.
 *
 * TODO:  Enable timeout with MCTP_BASE_PROTOCOL_MAX_RESPONSE_TIMEOUT_MS.  Do we really need to
 * though?
 */
const struct cmd_channel_i2c_dw_apb_slave system_i2c =
	cmd_channel_i2c_dw_apb_slave_static_init (&system_i2c_context, &i2c_hw, 0, 0);

/**
 * Variable context for the I2C slave driver.
 */
struct i2c_dw_apb_slave_state i2c_hw_context;

/**
 * I2C slave driver for managing the hardware interface.
 */
const struct i2c_dw_apb_slave i2c_hw = i2c_dw_apb_slave_static_init (&i2c_hw_context,
	(struct Creg_regs_DW_apb_i2c_APB_Slave*) HSP_ADDR_MAP_CREG_I2C0_ADDRESS,
	&system_i2c.i2c_handler);


/**
 * Initialize the slave mode I2C driver.
 *
 * @param i2c_slave_addr The I2C slave address
 * @param system_rx_buffers A pointer to an array of #cmd_packet used to receive I2C packets
 * @param system_rx_buffers_size The size of \p system_rx_buffers
 *
 * @return 0 if initialized successfully or an error code.
 */
int initialize_i2c_driver (uint8_t i2c_slave_addr, struct cmd_packet *system_rx_buffers,
	size_t system_rx_buffers_size)
{
	int status;

	status = i2c_dw_apb_slave_init_hw (&i2c_hw, i2c_slave_addr,	I2C_DW_APB_SPEED_FAST,
		HSP_CLOCK_FREQUENCY_HZ, MCTP_BASE_PROTOCOL_MAX_RESPONSE_TIMEOUT_MS * 5);
	if (status != 0) {
		goto done;
	}

	status = i2c_dw_apb_slave_init_state (&i2c_hw);
	if (status != 0) {
		goto done;
	}

	status = cmd_channel_i2c_dw_apb_slave_init_state (&system_i2c, system_rx_buffers,
		system_rx_buffers_size);
	if (status != 0) {
		goto done;
	}

done:

	return status;
}
