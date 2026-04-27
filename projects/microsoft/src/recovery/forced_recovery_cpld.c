// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "common/unused.h"
#include "recovery/forced_recovery_cpld.h"


/**
 * Read a register from the CPLD over I2C
 *
 * @param recovery The control interface to the CPLD to read the recovery bit.
 * @param cpld_reg_name The register address to read.
 * @param data The buffer to hold the register value.
 *
 * @return Transfer status, 0 if success or an error code.
 */
static int forced_recovery_cpld_read_register (const struct forced_recovery_cpld *recovery,
	uint8_t cpld_reg_name, uint8_t *data)
{
	return recovery->i2c->read_reg (recovery->i2c, recovery->slave_addr, cpld_reg_name, 1, data, 1);
}

/**
 * Check the CPLD recovery bit state and handle recovery.
 *
 * @param recovery The recovery instance that handles the recovery request.
 *
 * @return 0 if recovery processing is successful or an error code
 */
int forced_recovery_cpld_process_recovery (const struct forced_recovery_cpld *recovery)
{
	uint8_t bit;
	int status;

	if (recovery == NULL) {
		return FORCED_RECOVERY_CPLD_INVALID_ARGUMENT;
	}

	status = forced_recovery_cpld_read_register (recovery, FORCED_RECOVERY_CPLD_CTRL, &bit);
	if ((status == 0) && (bit == 1)) {
		status = recovery->irq->force_recovery (recovery->irq);
	}

	return status;
}

/**
 * Initialize a forced recovery instance.
 *
 * @param recovery The recovery instance that communicates with the CPLD to force recovery.
 * @param irq Handler for processing a forced recovery request.
 * @param i2c I2C device connected to the CPLD.
 * @param slave_addr The 7-bit I2C slave address of the CPLD.
 *
 * @return Initialization status, 0 if success or an error code.
 */
int forced_recovery_cpld_init (struct forced_recovery_cpld *recovery, struct host_irq_handler *irq,
	struct i2c_master_interface *i2c, uint8_t slave_addr)
{
	if ((recovery == NULL) || (irq == NULL) || (i2c == NULL)) {
		return FORCED_RECOVERY_CPLD_INVALID_ARGUMENT;
	}

	memset (recovery, 0, sizeof (struct forced_recovery_cpld));

	recovery->irq = irq;
	recovery->i2c = i2c;
	recovery->slave_addr = slave_addr;

	return 0;
}

/**
 * Release the resources used by forced recovery interface.
 *
 * @param cpld The forced recovery interface to release.
 */
void forced_recovery_cpld_release (struct forced_recovery_cpld *recovery)
{
	UNUSED (recovery);
}
