// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_CRASHDUMP_RAS_FAULT_INJECTION_STATIC_H_
#define SOC_CRASHDUMP_RAS_FAULT_INJECTION_STATIC_H_

#include "soc_crashdump_ras_fault_injection.h"


/* Internal functions declared to allow for static initialization. */
int soc_crashdump_ras_fault_injection_run_test (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info);

/**
 * Constant initializer for the RAS HSP fault injection API.
 */
#define	SOC_CRASHDUMP_RAS_FAULT_INJECTION_API_INIT  { \
		.run_self_test = soc_crashdump_ras_fault_injection_run_test, \
	}


/**
 * Initialize a static instance for on-demand RAS HSP faults injection test support using CMVP
 * testing. This can be a constant instance.
 *
 * There is no validation done on the arguments.
 * @param[in] mmio_creg_ptr The mmio creg reference.
 * @param[in] dmb_ptr The dmb reference.
 */
#define	soc_crashdump_ras_fault_injection_static_init(mmio_creg_ptr, dmb_ptr) { \
		.base = SOC_CRASHDUMP_RAS_FAULT_INJECTION_API_INIT, \
		.mmio_creg =  mmio_creg_ptr, \
		.dmb =  dmb_ptr, \
	}


#endif	/* SOC_CRASHDUMP_RAS_FAULT_INJECTION_STATIC_H_ */
