// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_WATCHDOG_STATIC_H_
#define HSP_WATCHDOG_STATIC_H_

#include "hsp_watchdog.h"


/**
 * Initialize a static instance for a timer to be used as a hardware watchdog.
 *
 * There is no validation done on the arguments.
 *
 * @param creg_ptr The CREG register interface (struct Creg_regs).
 * @param timer_offset_arg Offset within CREG for the timer registers to use as a watchdog
 * (struct Creg_regs_tmr).
 * @param fatal_err_offset_arg Offset within CREG for fatal error registers
 * (struct Creg_regs_fatal_err_log).
 */
#define	hsp_watchdog_static_init(creg_ptr, timer_offset_arg, fatal_err_offset_arg)	{ \
		.creg = creg_ptr, \
		.timer_regs = timer_offset_arg, \
		.fatal_err_regs = fatal_err_offset_arg, \
	}


#endif	/* HSP_WATCHDOG_STATIC_H_ */
