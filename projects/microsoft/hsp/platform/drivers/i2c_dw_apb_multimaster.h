// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_MULTIMASTER_H_
#define I2C_DW_APB_MULTIMASTER_H_

#include "drivers/i2c_dw_apb.h"
#include "drivers/i2c_dw_apb_multimaster_handler.h"


/**
 * Variable context for the I2C multi-master driver.
 */
struct i2c_dw_apb_multimaster_state {
	struct i2c_dw_apb_state state_base;	/**< Base driver state context. */
};

/**
 * Driver instance for supporting I2C multi-master operations using the DesignWare APB HW block from
 * Synopsis.
 */
struct i2c_dw_apb_multimaster {
	struct i2c_dw_apb i2c_base;	/**< Base I2C driver context. */
};


int i2c_dw_apb_multimaster_init (struct i2c_dw_apb_multimaster *i2c,
	struct i2c_dw_apb_multimaster_state *state, struct Creg_regs_DW_apb_i2c_APB_Slave *regs,
	const struct i2c_dw_apb_multimaster_handler *handler, uint8_t slave_address,
	enum i2c_dw_apb_speed mode, uint32_t ic_clk);
int i2c_dw_apb_multimaster_init_state (const struct i2c_dw_apb_multimaster *i2c);
void i2c_dw_apb_multimaster_release (const struct i2c_dw_apb_multimaster *i2c);

int i2c_dw_apb_multimaster_init_hw (const struct i2c_dw_apb_multimaster *i2c, uint8_t slave_address,
	enum i2c_dw_apb_speed mode, uint32_t ic_clk);


#endif	/* I2C_DW_APB_MULTIMASTER_H_ */
