// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FORCED_RECOVERY_CPLD_STATIC_H_
#define FORCED_RECOVERY_CPLD_STATIC_H_

#include "forced_recovery_cpld.h"


/**
 * Initialize a control instance to communicate with the CPLD to support forced host recovery.
 * This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param irq_ptr Handler for processing a forced recovery request.
 * @param i2c_ptr I2C device connected to the CPLD.
 * @param cpld_slave_addr The 7-bit I2C slave address of the CPLD.
 */
#define forced_recovery_cpld_static_init(irq_ptr, i2c_ptr, cpld_slave_addr) { \
		.irq = irq_ptr, \
		.i2c = i2c_ptr, \
		.slave_addr = cpld_slave_addr \
	}


#endif	// FORCED_RECOVERY_CPLD_STATIC_H_
