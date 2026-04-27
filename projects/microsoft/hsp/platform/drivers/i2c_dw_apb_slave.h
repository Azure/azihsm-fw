// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_SLAVE_H_
#define I2C_DW_APB_SLAVE_H_

#include "drivers/i2c_dw_apb.h"
#include "drivers/i2c_dw_apb_slave_handler.h"


/**
 * Driver instance for supporting I2C slave operations using the DesignWare APB HW block from
 * Synopsis.
 */
struct i2c_dw_apb_slave_state {
	struct i2c_dw_apb_state state_base;	/**< Base driver state context. */
	volatile unsigned txn_state;		/**< State machine of the current transaction. */
};

/**
 * Driver instance for supporting I2C slave operations using the DesignWare APB HW block from
 * Synopsis.
 */
struct i2c_dw_apb_slave {
	struct i2c_dw_apb i2c_base;	/**< Base I2C driver context. */
};


int i2c_dw_apb_slave_init (struct i2c_dw_apb_slave *i2c, struct i2c_dw_apb_slave_state *state,
	struct Creg_regs_DW_apb_i2c_APB_Slave *regs, const struct i2c_dw_apb_slave_handler *handler,
	uint16_t slave_address, enum i2c_dw_apb_speed mode, uint32_t ic_clk, unsigned scl_timeout_ms);
int i2c_dw_apb_slave_init_state (const struct i2c_dw_apb_slave *i2c);
void i2c_dw_apb_slave_release (const struct i2c_dw_apb_slave *i2c);

int i2c_dw_apb_slave_init_hw (const struct i2c_dw_apb_slave *i2c, uint16_t address,
	enum i2c_dw_apb_speed mode, uint32_t ic_clk, unsigned scl_timeout_ms);

int i2c_dw_apb_slave_begin_read_request_update (const struct i2c_dw_apb_slave *i2c,
	uintptr_t *critical);

size_t i2c_dw_apb_slave_discard_rx (const struct i2c_dw_apb_slave *i2c, uint8_t **data);
size_t i2c_dw_apb_slave_transmit_default_pad_bytes (const struct i2c_dw_apb_slave *i2c,
	const uint8_t **data);
int i2c_dw_apb_slave_pad_tx_with_default_byte (const struct i2c_dw_apb_slave *i2c);


#endif	/* I2C_DW_APB_SLAVE_H_ */
